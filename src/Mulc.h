#pragma once

#include "Mode.h"
#include "Flags.h"
#include "ProjectInfo.h"

#include <filesystem>

class Mulc
{
private:
    static Mode mode;
    static Flags flags;
    static ProjectInfo projectInfo;

    static std::filesystem::path builderPath;
    static std::filesystem::path initialPath;
    static std::filesystem::path buildFilePath;

    static std::filesystem::path binPath;

public:
    static void readArgs(int argc, char *argv[]);
    static void init(void);
    static void runScript(void);

private:
    enum class Type
    {
        APP,
        LIB,
        DLL
    };

    static void build(Type type, std::string path);

private:
    class ScriptAPI
    {
    private:
        static ProjectInfo *info;
    public:
        static void runScript(std::string script, ProjectInfo *info);

        static void group(std::string group);

        static void add_source(std::string source);
        static void remove_source(std::string source);

        static void std(std::string std);
        static void include_path(std::string includePath);
        static void define(std::string macro);
        static void compile_flag(std::string compileFlag);

        static void library(std::string lib);
        static void library_path(std::string libPath);
        static void named_library(std::string namedLib);
        static void link_flag(std::string linkFlag);

        static void require(std::string proj);

        static void export_files(std::string srcPath, std::string dstPath, bool clearDst = false);
        static void export_headers(std::string srcPath, std::string dstPath, bool clearDst = false);

        static void use_dependency(std::string dependency);

        static void dep_include_path(std::string includePath);
        static void dep_library(std::string lib);
        static void dep_library_path(std::string libPath);
        static void dep_named_library(std::string namedLib);

        static void build_app(std::string path);
        static void build_lib(std::string path);
        static void build_dll(std::string path);

        static void cmd(std::string cmd);
        static void msg(std::string msg);

        static std::string app(std::string name);
        static std::string lib(std::string name);
        static std::string dll(std::string name);
    };
};