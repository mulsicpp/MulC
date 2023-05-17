#include "PathStack.h"

#include <filesystem>

std::vector<std::string> PathStack::paths;

void PathStack::push(std::string path)
{
    paths.push_back(std::filesystem::canonical(std::filesystem::current_path()).string());
    std::filesystem::current_path(path);
}

void PathStack::pop(void)
{
    std::filesystem::current_path(paths.back());
    paths.pop_back();
}