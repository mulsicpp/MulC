#pragma once

#include "Mode.h"
#include "ProjectInfo.h"

struct SystemInterface
{
    struct MSVCInfo
    {
        std::string compilerPath = "";
        std::string systemIncludePaths = "";
        std::string systemLibPaths = "";
    } msvcInfo;

    int compile(const ProjectInfo::TranslationUnit &tu, ProjectInfo *buildInfo, Mode mode, std::string *output);

    int linkApp(ProjectInfo *buildInfo, Mode mode, std::string path, std::string *output);

    int linkDll(ProjectInfo *buildInfo, Mode mode, std::string path, std::string *output);

    int createLib(ProjectInfo *buildInfo, Mode mode, std::string path, std::string *output);

    int executeProgram(const char *prog, const char *args);
};