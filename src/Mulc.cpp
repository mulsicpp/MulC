#if defined(_WIN32)
#include "windows.h"
#endif

#if defined(__linux__)
#include <unistd.h>
#include <linux/limits.h>
#endif

#include "Mulc.h"
#include "utils.h"

#include "ScopePath.h"

#include <stdio.h>

#define EXISTS std::filesystem::exists
#define MOD_TIME std::filesystem::last_write_time

Mode Mulc::mode;
Flags Mulc::flags;
SystemInterface Mulc::systemInterface;

std::filesystem::path Mulc::builderPath = "";
std::filesystem::path Mulc::initialPath = "";

ProjectInfo *Mulc::ScriptAPI::info = nullptr;
std::vector<ProjectInfo *> Mulc::ScriptAPI::infoStack;

static bool isSubPath(const std::filesystem::path &base, const std::filesystem::path &sub)
{
    std::string relative = std::filesystem::relative(sub, base).string();
    // Size check for a "." result.
    // If the path starts with "..", it's not a subdirectory.
    return relative.size() == 1 || relative[0] != '.' && relative[1] != '.';
}

static bool addPathIfFree(std::vector<std::string> &list, std::string element)
{
    const auto &absolute = std::filesystem::absolute(element);
    for (const auto &el : list)
    {
        if (std::filesystem::absolute(el) == absolute)
            return false;
    }
#if defined(_WIN32)
    std::replace(element.begin(), element.end(), '/', '\\');
#endif
    list.push_back(element);
    return true;
}

static bool addElementIfFree(std::vector<std::string> &list, std::string element)
{
    for (const auto &el : list)
    {
        if (el == element)
            return false;
    }
    list.push_back(element);
    return true;
}

void Mulc::readArgs(int argc, char *argv[])
{
    mode.os = Mode::Os::OS;
    mode.arch = sizeof(void *) == 8 ? Mode::Arch::X64 : Mode::Arch::X86;
    mode.config = Mode::Config::RELEASE;

    if (mode.os == Mode::Os::UNKNOWN)
        error("This operating system is not supported");

    FlagIterator it = FlagIterator(argc, argv);

    // skip first parameter (program name)
    if (it.hasNext())
        it.next();

    if (it.hasNext())
    {
        const char *action = it.next();
        if (strcmp(action, "build") == 0)
            flags.action = Flags::Action::BUILD;
        else if (strcmp(action, "clear") == 0)
            flags.action = Flags::Action::CLEAR;
        else
            error("Action \'%s\' is not allowed. Has to be \'build\', \'setup\' or \'clear\'", action);
    }
    else
    {
        error("No action specified");
    }

A:
    while (it.hasNext())
    {
        auto flagName = it.next();
        for (auto flag : flags.flags)
            if (flag.process(&it))
                goto A;
        warning("Unknown flag \'%s\' ignored", flagName);
    }

    mode.arch = flags.arch;
    mode.config = flags.config;
}

void Mulc::init(void)
{
    initialPath = std::filesystem::canonical(std::filesystem::current_path());

#if defined(_WIN32)
    char result[MAX_PATH];
    GetModuleFileNameA(NULL, result, MAX_PATH);
#endif

#if defined(__linux__)
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
#endif
    builderPath = std::filesystem::canonical(std::filesystem::path(result).parent_path());

    setupMSVC();
}

void Mulc::runScript(void)
{
    ScriptAPI::runScript(flags.path);
}

void Mulc::ScriptAPI::build(Mulc::Type type, std::string path)
{
}

std::filesystem::path Mulc::ScriptAPI::getScriptPath(std::string path)
{
    std::string ret;
    if (std::filesystem::is_directory(path))
    {
        std::vector<std::filesystem::path> scriptFiles;

        auto iterator = std::filesystem::directory_iterator(path);
        for (const auto &entry : iterator)
        {
            if (entry.is_regular_file() && entry.path().extension() == ".mulc")
                scriptFiles.push_back(entry.path());
        }
        if (scriptFiles.size() == 1)
            ret = scriptFiles[0].string();
        else if (scriptFiles.size() == 0)
            error("No build script found");
        else
        {
            error("Build script is ambiguous");
        }
    }
    else
    {
        if (std::filesystem::path(path).extension() == ".mulc")
            ret = path;
        else
            error("The specified file \'%s\' is not a build script", path.c_str());
    }

    return std::filesystem::canonical(ret);
}

void Mulc::ScriptAPI::pushInfo(ProjectInfo *info)
{
    infoStack.push_back(info);
    bindInfo(infoStack.back());
}
void Mulc::ScriptAPI::popInfo(void)
{
    infoStack.pop_back();
    if (infoStack.size() > 0)
        bindInfo(infoStack.back());
    else
        bindInfo(nullptr);
}

void Mulc::ScriptAPI::bindInfo(ProjectInfo *info)
{
    ScriptAPI::info = info;
    if (info)
    {
        std::filesystem::current_path(info->buildFilePath.parent_path());
    }
    else
    {
        std::filesystem::current_path(initialPath);
    }
}

bool Mulc::findVcVarsAuto(SystemInterface::MSVCInfo *info)
{
    ScopePath p(initialPath);
    FILE *cmd;
    if ((cmd = _popen("set", "r")) == nullptr)
        return false;

    char buffer[8000];

    std::filesystem::path appdata, programfilesx64, programfilesx86;

    while (fgets(buffer, 8000, cmd))
    {
        if (buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = 0;
        std::string line(buffer);

        int offset = 0;
        if ((offset = line.find('=')) != std::string::npos)
        {
            if (line.substr(0, offset) == "APPDATA")
                appdata = line.substr(offset + 1);
            else if (line.substr(0, offset) == "ProgramFiles")
                programfilesx64 = line.substr(offset + 1);
            else if (line.substr(0, offset) == "ProgramFiles(x86)")
                programfilesx86 = line.substr(offset + 1);
        }
    }

    std::filesystem::path vcvarsPath;

    if (EXISTS(programfilesx64 / "Microsoft Visual Studio"))
        vcvarsPath = programfilesx64 / "Microsoft Visual Studio";
    else if (EXISTS(programfilesx86 / "Microsoft Visual Studio"))
        vcvarsPath = programfilesx86 / "Microsoft Visual Studio";
    else if (EXISTS(appdata / "Microsoft Visual Studio"))
        vcvarsPath = appdata / "Microsoft Visual Studio";
    else
        return false;

    std::filesystem::directory_iterator it(vcvarsPath);
    for (const auto entry : it)
        if (entry.exists())
            vcvarsPath = entry.path();

    it = std::filesystem::directory_iterator(vcvarsPath);
    for (const auto entry : it)
        if (entry.exists())
            vcvarsPath = entry.path();

    vcvarsPath = vcvarsPath / "VC" / "Auxiliary" / "Build";

    return createMSVCBuildInfo(info, vcvarsPath);
}

bool Mulc::findVcVarsInput(SystemInterface::MSVCInfo *info)
{
    ScopePath p(initialPath);

    char buffer[1024];
    printf("Please enter the folder containing the vcvars%s.bat script: ", mode.arch == Mode::Arch::X64 ? "64" : "86");
    if (!fgets(buffer, 1024, stdin))
        return false;

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = 0;

    return createMSVCBuildInfo(info, std::filesystem::canonical(buffer));
}

static std::vector<std::string> split(const std::string &str, char separator)
{
    int startIndex = 0, endIndex = 0;
    std::vector<std::string> strings;

    for (int i = 0; i <= str.size(); i++)
    {
        // If we reached the end of the word or the end of the input.
        if (str[i] == separator || i == str.size())
        {
            endIndex = i;
            std::string temp;
            temp.append(str, startIndex, endIndex - startIndex);
            strings.push_back(temp);
            startIndex = endIndex + 1;
        }
    }
    return strings;
}

bool Mulc::createMSVCBuildInfo(SystemInterface::MSVCInfo *info, std::filesystem::path path)
{
    ScopePath p(builderPath);

    if (!EXISTS(path / (mode.arch == Mode::Arch::X64 ? "vcvars64.bat" : "vcvars32.bat")))
        return false;
    FILE *cmd;
    if ((cmd = _popen((std::string("\"") + (path / (std::string("vcvars") + (mode.arch == Mode::Arch::X64 ? "64" : "32") + ".bat")).string() + "\" && SET").c_str(), "r")) == nullptr)
    {
        return false;
    }

    char buffer[8000];

    std::string
        env_VCToolsInstallDir,
        env_VSCMD_ARG_HOST_ARCH,
        env_VSCMD_ARG_TGT_ARCH,
        env_INCLUDE,
        env_LIB;

    while (fgets(buffer, 8000, cmd))
    {
        if (buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = 0;
        std::string line(buffer);

        std::string name;

        int offset = 0;
        if ((offset = line.find('=')) != std::string::npos)
        {
            name = line.substr(0, offset);
            if (name == "VCToolsInstallDir")
                env_VCToolsInstallDir = line.substr(offset + 1);
            else if (name == "VSCMD_ARG_HOST_ARCH")
                env_VSCMD_ARG_HOST_ARCH = line.substr(offset + 1);
            else if (name == "VSCMD_ARG_TGT_ARCH")
                env_VSCMD_ARG_TGT_ARCH = line.substr(offset + 1);
            else if (name == "INCLUDE")
                env_INCLUDE = line.substr(offset + 1);
            else if (name == "LIB")
                env_LIB = line.substr(offset + 1);
        }
    }

    if (_pclose(cmd) != 0)
        return false;

    systemInterface.msvcInfo.compilerPath = (std::filesystem::path(env_VCToolsInstallDir) / "bin" / ("Host" + env_VSCMD_ARG_HOST_ARCH) / env_VSCMD_ARG_TGT_ARCH).string();

    for (const auto &include : split(env_INCLUDE, ';'))
        systemInterface.msvcInfo.systemIncludePaths += OS_INCLUDE_PATH(std::filesystem::canonical(include).string());

    for (const auto &lib : split(env_LIB, ';'))
    {
        systemInterface.msvcInfo.systemLibPaths += OS_LIBRARY_PATH(std::filesystem::canonical(lib).string());
    }

    FILE *setup = fopen((mode.arch == Mode::Arch::X64 ? "msvcSetupx64" : "msvcSetupx86"), "w");

    if (setup)
    {
        fprintf(setup, "%s\n%s\n%s\n", systemInterface.msvcInfo.compilerPath.c_str(), systemInterface.msvcInfo.systemIncludePaths.c_str(), systemInterface.msvcInfo.systemLibPaths.c_str());
        fclose(setup);
    }
    /**/

    return true;
}

void Mulc::setupMSVC(void)
{
    ScopePath p(builderPath);

    if (!EXISTS(mode.arch == Mode::Arch::X64 ? "msvcSetupx64" : "msvcSetupx86"))
    {
        printf("Setting up compiler ...\n");
        if (!findVcVarsAuto(&systemInterface.msvcInfo))
        {
            printf("MSVC could not be found automatially\n");
            if (!findVcVarsInput(&systemInterface.msvcInfo))
                error("MSVC not found");
        }
    }
    else
    {
        FILE *setup = fopen((mode.arch == Mode::Arch::X64 ? "msvcSetupx64" : "msvcSetupx86"), "r");

        if (setup)
        {
            char buffer[3][8000];
            for (int i = 0; i < 3; i++)
            {
                fgets(buffer[i], 8000, setup);
                if (buffer[i][strlen(buffer[i]) - 1] == '\n')
                    buffer[i][strlen(buffer[i]) - 1] = 0;
            }

            systemInterface.msvcInfo.compilerPath = buffer[0];
            systemInterface.msvcInfo.systemIncludePaths = buffer[1];
            systemInterface.msvcInfo.systemLibPaths = buffer[2];

            fclose(setup);
        }
    }
    //printf("%s\n%s\n%s\n", systemInterface.msvcInfo.compilerPath.c_str(), systemInterface.msvcInfo.systemIncludePaths.c_str(), systemInterface.msvcInfo.systemLibPaths.c_str());
}

void Mulc::ScriptAPI::group(std::string group = "")
{
    if (group.size() > 0)
        info->group = group;
}

void Mulc::ScriptAPI::add_source(std::string source)
{
    std::filesystem::path sourcePath(source);
    if (!EXISTS(sourcePath))
        error("The source \'%s\' does not exist", sourcePath.string().c_str());
    if (!EXISTS(sourcePath))
    {
        warning("The source \'%s\' got ignored, because is not inside the project", sourcePath.string().c_str());
        return;
    }
    if (std::filesystem::is_directory(sourcePath))
    {
        auto iterator = std::filesystem::recursive_directory_iterator(sourcePath);
        for (const auto &entry : iterator)
        {
            if (entry.path().extension() == ".cpp" || entry.path().extension() == ".c")
                info->translationUnits.push_back({entry.path().string(), ""});
        }
    }
    else
    {
        info->translationUnits.push_back({sourcePath.string(), ""});
    }
}

void Mulc::ScriptAPI::remove_source(std::string source)
{
    std::filesystem::path sourcePath(source);
    if (!EXISTS(sourcePath))
        error("The source \'%s\' does not exist", sourcePath.string().c_str());
    if (std::filesystem::is_directory(sourcePath))
    {
        for (int i = 0; i < info->translationUnits.size();)
            if (isSubPath(sourcePath, info->translationUnits[i].cFilePath))
                info->translationUnits.erase(info->translationUnits.begin() + i);
            else
                i++;
    }
    else
    {
        sourcePath = std::filesystem::canonical(sourcePath);
        for (int i = 0; i < info->translationUnits.size();)
            if (sourcePath == std::filesystem::canonical(info->translationUnits[i].cFilePath))
                info->translationUnits.erase(info->translationUnits.begin() + i);
            else
                i++;
    }
}

void Mulc::ScriptAPI::std(std::string std)
{
    info->compileFlags += OS_STD(std);
}

void Mulc::ScriptAPI::include_path(std::string includePath)
{
    if (addPathIfFree(info->includePaths, includePath))
        info->compileFlags += OS_INCLUDE_PATH(includePath);
}

void Mulc::ScriptAPI::define(std::string macro)
{
    info->compileFlags += OS_DEFINE(macro);
}

void Mulc::ScriptAPI::compile_flag(std::string compileFlag)
{
    info->compileFlags += compileFlag + " ";
}

void Mulc::ScriptAPI::library(std::string lib)
{
    addPathIfFree(info->libs, lib);
}

void Mulc::ScriptAPI::library_path(std::string libPath)
{
    if (addPathIfFree(info->libPaths, libPath))
        info->linkerFlags += OS_LIBRARY_PATH(libPath);
}

void Mulc::ScriptAPI::named_library(std::string namedLib)
{
    addElementIfFree(info->libs, OS_NAMED_LIBRARY(namedLib));
}

void Mulc::ScriptAPI::link_flag(std::string linkFlag)
{
    info->linkerFlags += linkFlag + " ";
}

void Mulc::ScriptAPI::require(std::string proj)
{
    ScriptAPI::runScript(proj);
}

void Mulc::ScriptAPI::export_files(std::string srcPath, std::string dstPath, bool clearDst)
{
    using CopyOptions = std::filesystem::copy_options;

    auto co = CopyOptions::recursive | (clearDst ? CopyOptions::overwrite_existing : CopyOptions::skip_existing);
    if (clearDst && EXISTS(dstPath))
    {
        if (isSubPath(srcPath, dstPath))
        {
            warning("Could not export \'%s\' with cleared destination, because the destination is inside the source", srcPath.c_str());
            return;
        }
        if (isSubPath(dstPath, srcPath))
        {
            warning("Could not export \'%s\' with cleared destination, because the source is inside the destination", srcPath.c_str());
            return;
        }
        std::filesystem::remove_all(dstPath);
    }

    if (EXISTS(srcPath))
    {
        std::filesystem::create_directories(dstPath);
        std::filesystem::copy(srcPath, dstPath, co);
        printf(F_CYAN "Exported \'%s\' to \'%s\'\n" F_RESET, srcPath.c_str(), dstPath.c_str());
    }
    else
        warning("Could not export \'%s\', because it does not exist", srcPath.c_str());
}

void Mulc::ScriptAPI::export_headers(std::string srcPath, std::string dstPath, bool clearDst)
{
    using CopyOptions = std::filesystem::copy_options;

    auto co = CopyOptions::recursive | (clearDst ? CopyOptions::overwrite_existing : CopyOptions::skip_existing);
    if (clearDst && EXISTS(dstPath))
    {
        if (isSubPath(srcPath, dstPath))
        {
            warning("Could not export headers in \'%s\' with cleared destination, because the destination is inside the source", srcPath.c_str());
            return;
        }
        if (isSubPath(srcPath, dstPath))
        {
            warning("Could not export headers in \'%s\' with cleared destination, because the destination is inside the source", srcPath.c_str());
            return;
        }
        std::filesystem::remove_all(dstPath);
    }

    if (!EXISTS(srcPath))
    {
        warning("Could not export headers, because the folder \'%s\' does not exist", srcPath.c_str());
        return;
    }
    if (!std::filesystem::is_directory(srcPath))
    {
        warning("Could not export headers, because \'%s\' is not a folder", srcPath.c_str());
        return;
    }
    auto iterator = std::filesystem::recursive_directory_iterator(srcPath);
    for (const auto &entry : iterator)
    {
        if (entry.is_regular_file() && (entry.path().extension() == ".h" || entry.path().extension() == ".hpp"))
        {
            auto dstFile = std::filesystem::path(dstPath) / std::filesystem::proximate(entry.path(), srcPath).parent_path();
            std::filesystem::create_directories(dstFile);
            std::filesystem::copy(entry.path(), std::filesystem::canonical(dstFile) / entry.path().filename(), co);
        }
    }
    printf(F_CYAN "Exported headers from \'%s\' to \'%s\'\n" F_RESET, srcPath.c_str(), dstPath.c_str());
}

void Mulc::ScriptAPI::export_files_std(std::string srcPath, std::string dstPath)
{
    export_files(srcPath, dstPath, false);
}
void Mulc::ScriptAPI::export_headers_std(std::string srcPath, std::string dstPath)
{
    export_headers(srcPath, dstPath, false);
}

void Mulc::ScriptAPI::build_app(std::string path)
{
    build(Type::APP, path);
}

void Mulc::ScriptAPI::build_lib(std::string path)
{
    build(Type::LIB, path);
}

void Mulc::ScriptAPI::build_dll(std::string path)
{
    build(Type::DLL, path);
}

void Mulc::ScriptAPI::cmd(std::string cmd)
{
    printf("Running command " F_BLUE "%s" F_RESET ":\n\n", cmd.c_str());
    int ret = system(cmd.c_str());
    printf("\nCommand returned with %i\n", ret);
}

void Mulc::ScriptAPI::msg(std::string msg)
{
    printf(F_BOLD "[%s]: %s\n" F_RESET, info->buildFilePath.filename().string().c_str(), msg.c_str());
}

std::string app(std::string name)
{
#if defined(_WIN32)
    return name + ".exe";
#elif defined(__linux__)
    return name;
#endif
}

std::string lib(std::string name)
{
#if defined(_WIN32)
    return name + ".lib";
#elif defined(__linux__)
    return "lib" + name + ".a";
#endif
}

std::string dll(std::string name)
{
#if defined(_WIN32)
    return name + ".dll";
#elif defined(__linux__)
    return "lib" + name + ".so";
#endif
}
