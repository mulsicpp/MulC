#pragma once

#include <filesystem>

class ScopePath {
private:
    std::filesystem::path prevPath;
public:
    ScopePath(const std::filesystem::path &path);
    ~ScopePath();
};