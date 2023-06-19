<!-- omit in toc -->
# MulC
A build tool for C/C++

- [Building](#building)
  - [On Windows](#on-windows)
  - [On Linux](#on-linux)
- [Setting up your project](#setting-up-your-project)
- [Command Line Interface](#command-line-interface)
  - [Build flags](#build-flags)
    - [**Build script path**](#build-script-path)
    - [**Configuration**](#configuration)
    - [**Architecture**](#architecture)
    - [**Run application after building**](#run-application-after-building)
    - [**Force build**](#force-build)
    - [**Add variables**](#add-variables)
    - [**Setup MSVC compiler**](#setup-msvc-compiler)
    - [**Setup MSVC compiler with specific vcvars scripts**](#setup-msvc-compiler-with-specific-vcvars-scripts)
  - [Clean flags](#clean-flags)
    - [**Select group**](#select-group)
- [Scripting API](#scripting-api)


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
export PATH=$PATH:<path_to_mulc_directory>
```

# Setting up your project

To be able build one of your projects, you will need to create a build script in the project root directory. You can create multiple build scripts and call them whatever you want, but they need to have the `.mulc` file extension to be recognized by the builder.

The scripting language used is [Chaiscript](https://github.com/ChaiScript/ChaiScript). It is an open-source embedded scripting language designed specifically for C++. Its syntax is very similar to C++ and JavaScript, so variables, if statements and loops work as expected. The interface, that you should use in the script is specified in the [Scriping API](#api) chapter.

# Command Line Interface
You can use the builder in the command line with the following structure:
```
mulc [build|clean] [flag...]
```
The first parameter is the action, that the builder should perform. `build` tells the builder to build a project, while `clean` tells it to get rid of files created by past builds. With this you can already build a project, but you can also add flags for further control.

There are a number of flags that can be specified. They can be specified in any order, without changing their effect.

## Build flags

### **Build script path**
```
--path <path> 
```
or
```
-p <path> 
```
Sets the path to the build script, that should be used. The path can also be a directory, if it only contains a single build script, which will be detected automatically.

The default value for this flag is the current working directory.

### **Configuration**
```
--config <config>
```
or
```
-c <config>
```
Sets the build configuration for the project. The value passed can be either `debug` or `release`.

The default value for this flag is `release`.

### **Architecture**
```
--arch <arch>
```
or
```
-a <arch>
```
Sets the architecture, that the project should be build for. The value passed can be either `x86` or `x64`.

The default value for this flag is the architecture of the current machine.

### **Run application after building**
```
--run
```
or
```
-r
```
If the projects output is an application, it will be executed after successful building. This flag has no effect on other output types.

### **Force build**
```
--force
```
or
```
-f
```
Forces the compilation of all source files.

### **Add variables**
```
--var <name>=<value>
```
or
```
-v <name>=<value>
```
Creates a variable with the specified name and value. This variable can then be accessed in the build script.

### **Setup MSVC compiler**
```
--setup-msvc 
```
This flag is only relevant for Windows. This flag searches the machine for the MSVC compiler and stores some data to be able to use it right.

### **Setup MSVC compiler with specific vcvars scripts**
```
--setup-msvc-from <path>
```
Similar to `--mscv-setup`, but the builder does not search for a compiler. Instead it sets up the compiler by using the `vcvars***.bat` files in the specified directory.

## Clean flags

### **Select group**
```
--group <group>
```
Only deletes the created files of the specified group, instead of all of them. !!!

# Scripting API


