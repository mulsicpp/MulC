#include "ScopePath.h"

ScopePath::ScopePath(const std::filesystem::path &path) {
    prevPath = std::filesystem::canonical(std::filesystem::current_path());
    std::filesystem::current_path(path);
}

ScopePath::~ScopePath() {
    std::filesystem::current_path(prevPath);
}