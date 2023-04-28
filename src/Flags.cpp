#include "Flags.h"
#include "utils.h"

#include <string.h>

FlagIterator::FlagIterator(int argc, char **argv) : argc(argc), argv(argv), index(-1) {}

bool FlagIterator::hasNext(void)
{
    return index + 1 < argc;
}

char *FlagIterator::next()
{
    if (++index < argc)
        return argv[index];
    return nullptr;
}

char *FlagIterator::current()
{
    if (index < argc)
        return argv[index];
    return nullptr;
}

Flag::Flag(std::vector<const char *> names, std::function<void(void)> func)
{
    processor = [names, func](FlagIterator *it) -> bool
    {
        auto flag = it->current();
        for (auto name : names)
            if (strcmp(name, flag) == 0)
            {
                func();
                return true;
            }
        return false;
    };
}

Flag::Flag(std::vector<const char *> names, std::function<void(char *)> func)
{
    processor = [names, func](FlagIterator *it) -> bool
    {
        auto flag = it->current();
        for (auto name : names)
            if (strcmp(name, flag) == 0)
            {
                auto param = it->next();
                if(param == nullptr)
                    error("Flag \'%s\' requires a value", flag);
                func(param);
                return true;
            }
        return false;
    };
}

bool Flag::process(FlagIterator *it)
{
    return processor(it);
}