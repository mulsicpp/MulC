#include <stdio.h>

#include "Mulc.h"

int main(int argc, char *argv[])
{
    Mulc::readArgs(argc, argv);
    
    Mulc::init();
    Mulc::runScript();

    return 0;
}