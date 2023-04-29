#include "Mulc.h"

#include <chaiscript/chaiscript.hpp>

#define ADD_CHAI_FUNCTION(chai, x) chai.add(chaiscript::fun(&x), #x)

void Mulc::ScriptAPI::runScript(void)
{
    chaiscript::ChaiScript chai;

    //ADD_CHAI_FUNCTION(chai, output);
    //ADD_CHAI_FUNCTION(chai, group);
    //ADD_CHAI_FUNCTION(chai, add_source);
    //ADD_CHAI_FUNCTION(chai, remove_source);
    //ADD_CHAI_FUNCTION(chai, std);
    //ADD_CHAI_FUNCTION(chai, include_path);
    //ADD_CHAI_FUNCTION(chai, define);
    //ADD_CHAI_FUNCTION(chai, compile_flag);
    //ADD_CHAI_FUNCTION(chai, library);
    //ADD_CHAI_FUNCTION(chai, library_path);
    //ADD_CHAI_FUNCTION(chai, named_library);
    //ADD_CHAI_FUNCTION(chai, link_flag);
    //ADD_CHAI_FUNCTION(chai, require);
    //ADD_CHAI_FUNCTION(chai, export_files);
    //ADD_CHAI_FUNCTION(chai, export_headers);
    //ADD_CHAI_FUNCTION(chai, use_dependency);
    //ADD_CHAI_FUNCTION(chai, dep_include_path);
    //ADD_CHAI_FUNCTION(chai, dep_library);
    //ADD_CHAI_FUNCTION(chai, dep_library_path);
    //ADD_CHAI_FUNCTION(chai, dep_named_library);
    //ADD_CHAI_FUNCTION(chai, build);
    //ADD_CHAI_FUNCTION(chai, cmd);
    //ADD_CHAI_FUNCTION(chai, msg);
}

void Mulc::ScriptAPI::output(std::string path, std::string type)
{
}

void Mulc::ScriptAPI::group(std::string group)
{
}

void Mulc::ScriptAPI::add_source(std::string source)
{
}

void Mulc::ScriptAPI::remove_source(std::string source)
{
}

void Mulc::ScriptAPI::std(std::string std)
{
}

void Mulc::ScriptAPI::include_path(std::string includePath)
{
}

void Mulc::ScriptAPI::define(std::string macro)
{
}

void Mulc::ScriptAPI::compile_flag(std::string compileFlag)
{
}

void Mulc::ScriptAPI::library(std::string lib)
{
}

void Mulc::ScriptAPI::library_path(std::string libPath)
{
}

void Mulc::ScriptAPI::named_library(std::string namedLib)
{
}

void Mulc::ScriptAPI::link_flag(std::string linkFlag)
{
}

void Mulc::ScriptAPI::require(std::string proj)
{
}

void Mulc::ScriptAPI::export_files(std::string srcPath, std::string dstPath)
{
}

void Mulc::ScriptAPI::export_headers(std::string srcPath, std::string dstPath, bool clearDst)
{
}

void Mulc::ScriptAPI::use_dependency(std::string dependency)
{
}

void Mulc::ScriptAPI::dep_include_path(std::string includePath)
{
}

void Mulc::ScriptAPI::dep_library(std::string lib)
{
}

void Mulc::ScriptAPI::dep_library_path(std::string libPath)
{
}

void Mulc::ScriptAPI::dep_named_library(std::string namedLib)
{
}

void Mulc::ScriptAPI::build(void)
{
}

void Mulc::ScriptAPI::cmd(std::string cmd)
{
}

void Mulc::ScriptAPI::msg(std::string msg)
{
}
