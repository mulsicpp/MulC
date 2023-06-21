#pragma once

#include "Mode.h"
#include "Flags.h"
#include "ProjectInfo.h"

#include "SystemInterface.h"

#include <filesystem>

class Mulc
{
private:
    static Mode mode;
    static Flags flags;
    static SystemInterface systemInterface;

    static std::filesystem::path builderPath;
    static std::filesystem::path initialPath;

public:
    static void readArgs(int argc, char *argv[]);
    static void init(void);
    static void runScript(void);

private:
#if defined(_WIN32)
    static bool findVcVarsAuto(SystemInterface::MSVCInfo *info);
    static bool findVcVarsInput(SystemInterface::MSVCInfo *info);
    static bool createMSVCBuildInfo(SystemInterface::MSVCInfo *info, std::filesystem::path path);
#endif
    static void setupMSVC(void);

private:
    enum class Type
    {
        APP,
        LIB,
        DLL
    };

    class ScriptAPI
    {
    private:
        static ProjectInfo *info;
        static std::vector<ProjectInfo *> infoStack;

        static void pushInfo(ProjectInfo *info);
        static void popInfo(void);
        static std::filesystem::path getScriptPath(std::string path);

        static void bindInfo(ProjectInfo *info);

        static void build(Type type, std::string path);

        static std::string generateCompileFootprint(void);

        static std::string loadCompileFootprint(void);
        static void saveCompileFootprint(std::string footprint);

        static void loadHeaderDependencies(void);
        static void saveHeaderDependencies(void);

        static bool tuNeedsUpdate(ProjectInfo::TranslationUnit tu);

    public:
        static void runScript(std::string script);

        static void addConst(std::string name, const std::string value);

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

        static void export_files(std::string srcPath, std::string dstPath, bool clearDst);
        static void export_headers(std::string srcPath, std::string dstPath, bool clearDst);

        static void export_files_std(std::string srcPath, std::string dstPath);
        static void export_headers_std(std::string srcPath, std::string dstPath);

        static void build_app(std::string path);
        static void build_lib(std::string path);
        static void build_dll(std::string path);

        static void cmd(std::string cmd);
        static void msg(std::string msg);

        static void packages(std::string packages);

        static void start_package(std::string package);
        static void finish_package(void);

        static void use_package(std::string package);

        static void package_include_path(std::string includePath);
        static void package_library(std::string lib);
        static void package_library_path(std::string libPath);
        static void package_named_library(std::string namedLib);

        static std::string app(std::string name);
        static std::string lib(std::string name);
        static std::string dll(std::string name);
    };
};