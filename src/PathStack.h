#pragma once

#include <vector>
#include <string>

class PathStack {
private:
    static std::vector<std::string> paths;
public:
    static void push(std::string path);
    static void pop(void);
};