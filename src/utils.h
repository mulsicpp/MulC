#pragma once

#define UPDATED_CODE 0x1
#define ERROR_CODE 0x2

#define F_RESET "\033[0m"

#define F_BLACK "\033[0;30m"
#define F_RED "\033[0;31m"
#define F_GREEN "\033[0;32m"
#define F_YELLOW "\033[0;33m"
#define F_BLUE "\033[0;34m"
#define F_PURPLE "\033[0;35m"
#define F_CYAN "\033[0;36m"
#define F_WHITE "\033[0;37m"
#define F_BOLD "\033[1m"

#include <stdio.h>
#include <stdlib.h>

template <class... Ts>
void error(const char *str, Ts... args)
{
    printf(F_RED F_BOLD "ERROR: " F_RESET F_RED);
    printf(str, args...);
    printf("\n" F_RESET);
    exit(-1);
}

template <class... Ts>
void syntaxError(const char *str, Ts... args)
{
    printf(F_RED F_BOLD "SYNTAX ERROR: " F_RESET F_RED);
    printf(str, args...);
    printf("\n" F_RESET);
    exit(-1);
}

template <class... Ts>
void warning(const char *str, Ts... args)
{
    printf(F_YELLOW F_BOLD "WARNING: " F_RESET F_YELLOW);
    printf(str, args...);
    printf("\n" F_RESET);
}

#if defined(_WIN32)

#define OS_NAME "windows"
#define OS_INCLUDE_PATH(x) "/I\"" + x + "\" "
#define OS_LIBRARY_PATH(x) "/LIBPATH:\"" + x + "\" "
#define OS_NAMED_LIBRARY(x) "\"" + x + ".lib\" "
#define OS_DEFINE(x) "/D\"" + x + "\" "
#define OS_STD(x) "/std:\"" + x + "\" "
#define OS_DIR_SEPARATOR "\\"

#define OS_APP(x) (x + ".exe")
#define OS_LIB(x) (x + ".lib")
#define OS_DLL(x) (x + ".dll")

#elif defined(__linux__)

#define OS_NAME "linux"
#define OS_INCLUDE_PATH(x) "-I\"" + x + "\" "
#define OS_LIBRARY_PATH(x) "-L\"" + x + "\" "
#define OS_NAMED_LIBRARY(x) "-l\"" + x + "\" "
#define OS_DEFINE(x) "-D\"" + x + "\" "
#define OS_STD(x) "-std=\"" + x + "\" "
#define OS_DIR_SEPARATOR "/"

#define OS_APP(x) x
#define OS_LIB(x) ("lib" + x + ".a")
#define OS_DLL(x) ("lib" + x + ".so")

#endif