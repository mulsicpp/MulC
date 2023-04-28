#pragma once

#include <string>
#include <functional>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include <sstream>
#include <iomanip>

#include "utils.h"

#include "Mode.h"

class FlagIterator
{
private:
    int argc;
    char **argv;

    int index;

public:
    FlagIterator(int argc, char **argv);

    bool hasNext(void);
    char *next(void);
    char *current(void);
};

class Flag
{
private:
    std::function<bool(FlagIterator *)> processor;

public:
    Flag(std::vector<const char *> names, std::function<void(void)> func);
    Flag(std::vector<const char *> names, std::function<void(char *)> func);

    bool process(FlagIterator *it);
};

struct Flags
{
    bool run = false;
    bool force = false;
    std::string path = ".";

    Mode::Arch arch = sizeof(void *) == 8 ? Mode::Arch::X64 : Mode::Arch::X86;
    Mode::Config config = Mode::Config::RELEASE;

    std::string group = "";

    enum class Action
    {
        BUILD,
        CLEAR
    } action = Action::BUILD;

    std::unordered_map<std::string, std::string> vars;

    std::stringstream childFlags;

    std::vector<Flag> flags = {
        Flag({"--run", "-r"}, [this](void)
             { this->run = true; }),
        Flag({"--force", "-f"}, [this](void)
             { this->force = true; childFlags << "--force "; }),

        Flag({"--path", "-p"}, [this](char *s)
             { this->path = s; }),

        Flag({"--arch", "-a"}, [this](char *s)
             { 
                if (strcmp(s, "x64") == 0) this->arch = Mode::Arch::X64; else if (strcmp(s, "x86") == 0) this->arch = Mode::Arch::X86; else error("The architecture \'%s\' is unknown", s); childFlags << "--arch " << s << " "; }),
        Flag({"--config", "-c"}, [this](char *s)
             { if (strcmp(s, "release") == 0) this->config = Mode::Config::RELEASE; else if (strcmp(s, "debug") == 0) this->config = Mode::Config::DEBUG; else error("The configuration \'%s\' is unknown", s); childFlags << "--config " << s << " "; }),

        Flag({"--var", "-v"}, [this](char *name)
             {
            childFlags << "--var " << std::quoted(name) << " ";
            char *value = strchr(name, '=');
            *value++ = 0;
            vars[name] = value; }),
        Flag({"--group", "-g"}, [this](char *s)
             { this->group = s; })};
};
