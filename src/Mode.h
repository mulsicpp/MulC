#pragma once

#if defined(_WIN32)
    #define OS WINDOWS
#elif defined(__linux__)
    #define OS LINUX
#else
    #define OS UNKNOWN
#endif

struct Mode {
    enum class Os {
        UNKNOWN,
        WINDOWS,
        LINUX
    } os;

    enum class Arch {
        UNKNOWN,
        X86,
        X64
    } arch;

    enum class Config {
        RELEASE,
        DEBUG
    } config;
};