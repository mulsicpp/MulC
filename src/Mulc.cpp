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

    printf("%i %i\n", flags.force, flags.run);
}

void Mulc::init(void)
{
}

void Mulc::runScript(void) {
    ScriptAPI::runScript();
}