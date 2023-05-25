#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

struct ProjectInfo
{
    struct TranslationUnit {
        std::string cFilePath;
        std::string oFilePath;
    };
    
    std::vector<TranslationUnit> translationUnits;

    std::string compileFlags;
    std::string linkerFlags;

    std::vector<std::string> includePaths;
    std::vector<std::string> libs;
    std::vector<std::string> libPaths;
    

    std::unordered_map<std::string, std::vector<std::string>> headerDependencies;

    std::filesystem::path buildFilePath;

    std::filesystem::path buildDir = std::filesystem::path("mulc.build") / "default";
};