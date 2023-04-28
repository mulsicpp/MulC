#pragma once

#include "Mode.h"
#include "Flags.h"
#include "ProjectInfo.h"

class Mulc {
private:
    static Mode mode;
    static Flags flags;
    static ProjectInfo projectInfo;

public:
    static void run(void);

private:
    static void init(void);
    static void readArgs(int argc, char *argv[]);
    static void runScript(void);
};