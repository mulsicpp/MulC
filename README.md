<!-- omit in toc -->
# MulC
A build tool for C/C++

- [Building](#building)
  - [On Windows](#on-windows)
  - [On Linux](#on-linux)


# Building

## On Windows
**Requirements:**
- Visual Studio 2017 or later (c++17 standard)
- CMake 3.10 or later

After you have installed the requirements, simply execute the `build.bat` script. The executable file will be located in the `windows` directory. You should add this directory to your `PATH` environment variable, so you can call `mulc` from anywhere.

## On Linux
**Requirements:**
- GCC 5
- Make

After you have installed the requirements, simply execute the `build.sh` script. The executable file will be located in the `linux` directory. Add the following line to your `~/.bashrc` file, so you can call `mulc` from anywhere:
```sh
export PATH=$PATH:<path_to_cppbuild_directory>
```