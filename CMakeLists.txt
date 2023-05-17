cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(mulc)

add_executable(mulc src/main.cpp src/Flags.cpp src/Mulc.cpp src/Mulc.runScript.cpp src/SystemInterface.cpp src/ScopePath.cpp)

target_precompile_headers(mulc PUBLIC ChaiScript/include/chaiscript/chaiscript.hpp)
target_include_directories(mulc PUBLIC ChaiScript/include)
