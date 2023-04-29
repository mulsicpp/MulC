#if defined(_WIN32)
#include "windows.h"
#endif

#if defined(__linux__)
#include <unistd.h>
#include <linux/limits.h>
#endif

#include "Mulc.h"

#include <stdio.h>

Mode Mulc::mode;
Flags Mulc::flags;
ProjectInfo Mulc::projectInfo;

std::filesystem::path Mulc::builderPath = "";
std::filesystem::path Mulc::initialPath = "";
std::filesystem::path Mulc::buildFilePath = "";

std::filesystem::path Mulc::binPath = "";

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
    ScriptAPI::runScript();
}