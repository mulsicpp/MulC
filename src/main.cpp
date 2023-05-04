#include <stdio.h>

#include "Mulc.h"

int main(int argc, char *argv[])
{
    try
    {
        Mulc::readArgs(argc, argv);

        Mulc::init();
        Mulc::runScript();
    }
    catch (std::exception e)
    {
        printf("%s\n", e.what());
    }

    return 0;
}