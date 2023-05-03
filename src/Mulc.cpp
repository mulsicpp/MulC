#if defined(_WIN32)
#include "windows.h"
#endif

#if defined(__linux__)
#include <unistd.h>
#include <linux/limits.h>
#endif

#include "Mulc.h"
#include "utils.h"

#include <stdio.h>

#define EXISTS std::filesystem::exists
#define MOD_TIME std::filesystem::last_write_time

Mode Mulc::mode;
Flags Mulc::flags;
ProjectInfo Mulc::projectInfo;

std::filesystem::path Mulc::builderPath = "";
std::filesystem::path Mulc::initialPath = "";
std::filesystem::path Mulc::buildFilePath = "";

std::filesystem::path Mulc::binPath = "";

ProjectInfo *Mulc::ScriptAPI::info;

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

    if (std::filesystem::is_directory(flags.path))
    {
        std::vector<std::filesystem::path> scriptFiles;

        auto iterator = std::filesystem::directory_iterator(flags.path);
        for (const auto &entry : iterator)
        {
            if (entry.is_regular_file() && entry.path().extension() == ".mulc")
                scriptFiles.push_back(entry.path());
        }
        if (scriptFiles.size() == 1)
            buildFilePath = scriptFiles[0].string();
        else if (scriptFiles.size() == 0)
            error("No build script found");
        else
        {
            error("Build script is ambiguous");
        }
    }
    else
    {
        if (std::filesystem::path(flags.path).extension() == ".mulc")
            buildFilePath = flags.path;
        else
            error("The specified file \'%s\' is not a build script", flags.path.c_str());
    }

    buildFilePath = std::filesystem::canonical(buildFilePath);
}

void Mulc::runScript(void)
{
    ScriptAPI::runScript(buildFilePath.string(), &projectInfo);
    printf("comp flags: %s\n", projectInfo.compileFlags.c_str());
}

void Mulc::build(Mulc::Type type, std::string path)
{
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
                info->translationUnits.push_back({entry.path().string(), (binPath / "bin" / entry.path()).string() + ".o"});
        }
    }
    else
    {
        info->translationUnits.push_back({sourcePath.string(), (binPath / "bin" / source).string() + ".o"});
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
        info->compileFlags += OS_LIBRARY_PATH(libPath);
}

void Mulc::ScriptAPI::named_library(std::string namedLib)
{
    addElementIfFree(info->libs, namedLib);
}

void Mulc::ScriptAPI::link_flag(std::string linkFlag)
{
    info->linkerFlags += linkFlag + " ";
}

void Mulc::ScriptAPI::require(std::string proj)
{
    ProjectInfo depInfo;

}

void Mulc::ScriptAPI::export_files(std::string srcPath, std::string dstPath, bool clearDst)
{
}

void Mulc::ScriptAPI::export_headers(std::string srcPath, std::string dstPath, bool clearDst)
{
}

void Mulc::ScriptAPI::use_dependency(std::string dependency)
{
}

void Mulc::ScriptAPI::dep_include_path(std::string includePath)
{
}

void Mulc::ScriptAPI::dep_library(std::string lib)
{
}

void Mulc::ScriptAPI::dep_library_path(std::string libPath)
{
}

void Mulc::ScriptAPI::dep_named_library(std::string namedLib)
{
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
}

void Mulc::ScriptAPI::msg(std::string msg)
{
}


std::string app(std::string name) {
#if defined(_WIN32)
    return name + ".exe";
#elif defined(__linux__)
    return name;
#endif
}

std::string lib(std::string name) {
#if defined(_WIN32)
    return name + ".lib";
#elif defined(__linux__)
    return "lib" + name + ".a";
#endif
}

std::string dll(std::string name) {
#if defined(_WIN32)
    return name + ".dll";
#elif defined(__linux__)
    return "lib" + name + ".so";
#endif
}
