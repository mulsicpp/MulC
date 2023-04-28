#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct ProjectInfo
{
    enum Type
    {
        APP,
        LIB,
        DLL
    } type;

    std::string path;

    std::string group;

    struct TranslationUnit
    {
        std::string cFilePath;
        std::string oFilePath;
    };

    std::vector<TranslationUnit> translationUnits;

    std::string compileFlags;
    std::string linkerFlags;

    std::vector<std::string> includePaths;
    std::string libs;

    struct ExportDependencies
    {
        std::vector<std::string> includePaths;
        std::vector<std::string> libPaths;
        std::vector<std::string> libs;
        std::vector<std::string> namedLibs;
    } exportDependencies;
    

    std::unordered_map<std::string, std::vector<std::string>> headerDependencies;
};