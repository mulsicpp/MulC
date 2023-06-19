#include "Mulc.h"

#include <chaiscript/chaiscript.hpp>

#include <list>

#define ADD_CHAI_FUNCTION(chai, x) chai.add(chaiscript::fun(&x), #x)
#define ADD_CHAI_FUNCTION_MULTI(chai, x) chai.add(chaiscript::fun([](std::list<std::string> strs) { callForEach(&x, strs); }), #x)

#define ADD_CHAI_FUNCTION_NAMED(chai, x, name) chai.add(chaiscript::fun(&x), name)

typedef void (*iterable_fun)(std::string);

static void callForEach(iterable_fun f, std::list<std::string> strs)
{
    for (std::string str : strs)
        f(str);
}

static std::list<std::string> combine_string_string(const std::string &str1, const std::string &str2)
{
    return std::list<std::string>{str1, str2};
}

static std::list<std::string> combine_list_string(const std::list<std::string> &strs, const std::string &str)
{
    auto ret = strs;
    ret.push_back(str);
    return ret;
}

static std::list<std::string> combine_string_list(const std::string &str, const std::list<std::string> &strs)
{
    auto ret = strs;
    ret.push_front(str);
    return ret;
}

static std::list<std::string> combine_list_list(const std::list<std::string> &strs1, std::list<std::string> strs2)
{
    auto ret = strs1;
    ret.merge(strs2);
    return ret;
}

void Mulc::ScriptAPI::runScript(std::string script)
{
    ProjectInfo currentInfo;

    currentInfo.buildFilePath = getScriptPath(script);
    pushInfo(&currentInfo);

    chaiscript::ChaiScript chai;

    currentInfo.chai = &chai;

    ADD_CHAI_FUNCTION(chai, group);
    ADD_CHAI_FUNCTION(chai, add_source);
    ADD_CHAI_FUNCTION(chai, remove_source);
    ADD_CHAI_FUNCTION(chai, std);
    ADD_CHAI_FUNCTION(chai, include_path);
    ADD_CHAI_FUNCTION(chai, define);
    ADD_CHAI_FUNCTION(chai, compile_flag);
    ADD_CHAI_FUNCTION(chai, library);
    ADD_CHAI_FUNCTION(chai, library_path);
    ADD_CHAI_FUNCTION(chai, named_library);
    ADD_CHAI_FUNCTION(chai, link_flag);
    ADD_CHAI_FUNCTION(chai, require);
    ADD_CHAI_FUNCTION(chai, export_files);
    ADD_CHAI_FUNCTION(chai, export_headers);
    ADD_CHAI_FUNCTION(chai, build_app);
    ADD_CHAI_FUNCTION(chai, build_lib);
    ADD_CHAI_FUNCTION(chai, build_dll);
    ADD_CHAI_FUNCTION(chai, cmd);
    ADD_CHAI_FUNCTION(chai, msg);

    ADD_CHAI_FUNCTION(chai, app);
    ADD_CHAI_FUNCTION(chai, lib);
    ADD_CHAI_FUNCTION(chai, dll);

    ADD_CHAI_FUNCTION_MULTI(chai, add_source);
    ADD_CHAI_FUNCTION_MULTI(chai, remove_source);
    ADD_CHAI_FUNCTION_MULTI(chai, std);
    ADD_CHAI_FUNCTION_MULTI(chai, include_path);
    ADD_CHAI_FUNCTION_MULTI(chai, define);
    ADD_CHAI_FUNCTION_MULTI(chai, compile_flag);
    ADD_CHAI_FUNCTION_MULTI(chai, library);
    ADD_CHAI_FUNCTION_MULTI(chai, library_path);
    ADD_CHAI_FUNCTION_MULTI(chai, named_library);
    ADD_CHAI_FUNCTION_MULTI(chai, link_flag);
    ADD_CHAI_FUNCTION_MULTI(chai, require);
    ADD_CHAI_FUNCTION_MULTI(chai, cmd);
    ADD_CHAI_FUNCTION_MULTI(chai, msg);

    ADD_CHAI_FUNCTION_NAMED(chai, export_files_std, "export_files");
    ADD_CHAI_FUNCTION_NAMED(chai, export_headers_std, "export_headers");

    ADD_CHAI_FUNCTION_NAMED(chai, combine_string_string, "&");
    ADD_CHAI_FUNCTION_NAMED(chai, combine_list_string, "&");
    ADD_CHAI_FUNCTION_NAMED(chai, combine_string_list, "&");
    ADD_CHAI_FUNCTION_NAMED(chai, combine_list_list, "&");

    for(const auto &[key, val] : flags.vars)
        chai.add(chaiscript::const_var<std::string>(val), key);

    chai.add(chaiscript::const_var<std::string>(OS_NAME), "OS");
    chai.add(chaiscript::const_var<std::string>(mode.arch == Mode::Arch::X64 ? "x64" : mode.arch == Mode::Arch::X86 ? "x86" : "unknown"), "ARCH");
    chai.add(chaiscript::const_var<std::string>(mode.config == Mode::Config::RELEASE ? "release" : "debug"), "CONFIG");
    chai.add(chaiscript::const_var<std::string>(currentInfo.buildDir.string()), "BUILD_DIR");

    try
    {
        chai.eval_file(info->buildFilePath.filename().string());
    }
    catch (chaiscript::exception::eval_error e)
    {
        printf("reason: %s, detail: %s, what: %s\n", e.reason.c_str(), e.detail.c_str(), e.what());
    }

    popInfo();
}

void Mulc::ScriptAPI::addConst(std::string name, const std::string value) {
    ((chaiscript::ChaiScript *)info->chai)->add(chaiscript::const_var<std::string>(value), name);
}
