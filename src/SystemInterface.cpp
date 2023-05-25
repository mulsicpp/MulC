#if defined(_WIN32)
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#elif defined(__linux__)
#include <unistd.h>
#include <linux/limits.h>
#endif

#include "SystemInterface.h"
#include "utils.h"

#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <string.h>

using Os = Mode::Os;
using Arch = Mode::Arch;
using Config = Mode::Config;

#if defined(_WIN32)

//
//
// TODO: Check all functions !!!
//
//

static FILE *win_popen(const char *proc, const char *args, HANDLE *out, PROCESS_INFORMATION *pi)
{
    HANDLE outWrite = NULL;

    SECURITY_ATTRIBUTES saAttr;

    // Set the bInheritHandle flag so pipe handles are inherited.

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.

    if (!CreatePipe(out, &outWrite, &saAttr, 0))
        exit(-1);

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if (!SetHandleInformation(*out, HANDLE_FLAG_INHERIT, 0))
        exit(-1);

    STARTUPINFOA si;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure.

    ZeroMemory(pi, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure.
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = outWrite;
    si.hStdOutput = outWrite;
    si.hStdInput = NULL;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process.

    bSuccess = CreateProcessA(proc,
                              (char *)args, // command line
                              NULL,         // process security attributes
                              NULL,         // primary thread security attributes
                              TRUE,         // handles are inherited
                              0,            // creation flags
                              NULL,         // use parent's environment
                              NULL,         // use parent's current directory
                              &si,          // STARTUPINFO pointer
                              pi);          // receives PROCESS_INFORMATION

    if (bSuccess)
    {
        CloseHandle(outWrite);

        int nHandle = _open_osfhandle((long)*out, _O_RDONLY);

        if (nHandle != -1)
        {
            FILE *p_file = _fdopen(nHandle, "r");
            return p_file;
        }
    }
    return NULL;
}

static int win_pclose(HANDLE *out, PROCESS_INFORMATION *pi)
{
    // Wait for the process to exit
    WaitForSingleObject(pi->hProcess, INFINITE);

    // Process has exited - check its exit code
    DWORD exitCode;
    GetExitCodeProcess(pi->hProcess, &exitCode);

    // At this point exitCode is set to the process' exit code

    // Handles must be closed when they are no longer needed
    CloseHandle(pi->hThread);
    CloseHandle(pi->hProcess);

    CloseHandle(*out);

    return exitCode;
}

#endif

static std::string &trim(std::string &str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch)
                                        { return !std::isspace(ch); }));

    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch)
                           { return !std::isspace(ch); })
                  .base(),
              str.end());

    return str;
}

int SystemInterface::compile(const ProjectInfo::TranslationUnit &tu, ProjectInfo *buildInfo, Mode mode, std::string *output)
{
    char buffer[1024];

    const unsigned int pref_length = 21;
    char *path;
    std::filesystem::path current_include;
    int include_length = 0;

    std::vector<std::string> headers;
#if defined(_WIN32)
    HANDLE out;
    PROCESS_INFORMATION pi;

    std::string str;

    std::string prog = msvcInfo.compilerPath + "\\cl.exe";
    std::string args = std::string(mode.config == Config::DEBUG ? " /DEBUG:FASTLINK " : " ") + buildInfo->compileFlags + " " + msvcInfo.systemIncludePaths + " /c /showIncludes /EHsc /MD /Fo\"" + tu.oFilePath + "\" \"" + tu.cFilePath + "\"";

    FILE *pipe = win_popen(prog.c_str(), args.c_str(), &out, &pi);

    if (!pipe)
    {
        return ERROR_CODE;
    }

    for (int i = 0; i < 3; i++)
        fgets(buffer, 1024, pipe);

    while (fgets(buffer, 1024, pipe))
    {
        if (memcmp(buffer, "Note: including file:", pref_length) == 0)
        {
            path = buffer + 21;

            while ((++path)[0] == ' ')
                ;
            include_length = strlen(path);
            while (path[include_length - 1] == '\n' || path[include_length - 1] == '\r')
            {
                path[include_length - 1] = 0;
                include_length = strlen(path);
            }

            auto rel = std::filesystem::relative(path, std::filesystem::current_path());
            if (!rel.empty() && rel.native()[0] != '.')
            {
                headers.push_back(std::filesystem::proximate(path).string());
                continue;
            }
            for (const auto &include_Path : buildInfo->includePaths)
            {
                auto rel = std::filesystem::relative(path, include_Path);
                if (!rel.empty() && rel.native()[0] != '.')
                {
                    headers.push_back(std::filesystem::proximate(path).string());
                    break;
                }
            }
        }
        else
        {
            str = buffer;
            trim(str);
            if (str != std::filesystem::path(tu.cFilePath).filename())
                *output += buffer;
        }
    }

    buildInfo->headerDependencies[tu.cFilePath] = headers;

    return win_pclose(&out, &pi) == 0 ? UPDATED_CODE : ERROR_CODE;
#elif defined(__linux__)
    FILE *pipe = popen(("g++ -march=x86-64 " + std::string(mode.arch == Arch::X86 ? "-m32 " : "-m64 ") + buildInfo->compileFlags + "-c -H -o \"" + tu.oFilePath + "\" \"" + tu.cFilePath + "\" 2>&1").c_str(), "r");

    if (!pipe)
    {
        return ERROR_CODE;
    }

    while (fgets(buffer, 1024, pipe))
    {
        // check include files
        if (buffer[0] == '.')
        {
            path = buffer + 1;
            while (path[0] == '.')
                path++;
            if (path[0] == ' ')
            {
                path++;
                include_length = strlen(path);
                path[include_length - 1] = 0;
                auto rel = std::filesystem::relative(path, std::filesystem::current_path());
                if (!rel.empty() && rel.native()[0] != '.')
                {
                    headers.push_back(std::filesystem::proximate(path).string());
                    continue;
                }
                for (const auto &include_Path : buildInfo->includePaths)
                {
                    auto rel = std::filesystem::relative(path, include_Path);
                    if (!rel.empty() && rel.native()[0] != '.')
                    {
                        headers.push_back(std::filesystem::proximate(path).string());
                        break;
                    }
                }
            }
        }
        else if (strcmp(buffer, "Multiple include guards may be useful for:\n") == 0)
        {
            continue;
        }
        else
        {
            if (buffer[0] != '/')
            {
                *output += buffer;
            }
        }
    }

    buildInfo->headerDependencies[tu.cFilePath] = headers;

    return pclose(pipe) == 0 ? UPDATED_CODE : ERROR_CODE;
#endif
}

int SystemInterface::linkApp(ProjectInfo *buildInfo, Mode mode, std::string path, std::string *output)
{
    std::string oFiles = "";
    for (const auto &tu : buildInfo->translationUnits)
        oFiles += "\"" + tu.oFilePath + "\" ";

    std::string libStr = "";
    for (const auto &lib : buildInfo->libs)
        libStr += "\"" + lib + "\" ";

    char buffer[1024];
#if defined(_WIN32)
    HANDLE out;
    PROCESS_INFORMATION pi;
    std::string prog = msvcInfo.compilerPath + "\\link.exe";
    std::string args = std::string(mode.config == Config::DEBUG ? " /DEBUG:FASTLINK " : " ") + "/out:\"" + path + "\" " + oFiles + buildInfo->linkerFlags  + libStr + msvcInfo.systemLibPaths;
    FILE *pipe = win_popen(prog.c_str(), args.c_str(), &out, &pi);
#elif defined(__linux__)
    FILE *pipe = popen(("g++ -march=x86-64 " + std::string(mode.arch == Arch::X86 ? "-m32 " : "-m64 ") + "-o \"" + path + "\" " + oFiles + "-Wl,--start-group " + buildInfo->linkerFlags + libStr + " -Wl,--end-group 2>&1").c_str(), "r");
#endif
    if (!pipe)
    {
        return ERROR_CODE;
    }

#if defined(_WIN32)
    for (int i = 0; i < 3; i++)
        fgets(buffer, 1024, pipe);
#endif

    while (fgets(buffer, 1024, pipe))
        *output += buffer;
#if defined(_WIN32)
    return win_pclose(&out, &pi) == 0 ? UPDATED_CODE : ERROR_CODE;
#elif defined(__linux__)
    return pclose(pipe) == 0 ? UPDATED_CODE : ERROR_CODE;
#endif
}

int SystemInterface::createLib(ProjectInfo *buildInfo, Mode mode, std::string path, std::string *output)
{
    char buffer[1024];
    std::string oFiles = "";
    for (const auto &tu : buildInfo->translationUnits)
        oFiles += "\"" + tu.oFilePath + "\" ";
#if defined(_WIN32)
    HANDLE out;
    PROCESS_INFORMATION pi;

    std::string prog = msvcInfo.compilerPath + "\\lib.exe";
    std::string args = " /out:\"" + path + "\" " + oFiles;

    FILE *pipe = win_popen(prog.c_str(), args.c_str(), &out, &pi);
#elif defined(__linux__)
    FILE *pipe = popen(("ar rcs \"" + path + "\" " + oFiles + " 2>&1").c_str(), "r");
#endif
    if (!pipe)
    {
        return ERROR_CODE;
    }

#if defined(_WIN32)
    for (int i = 0; i < 3; i++)
        fgets(buffer, 1024, pipe);
#endif

    while (fgets(buffer, 1024, pipe))
        *output += buffer;
#if defined(_WIN32)
    return win_pclose(&out, &pi) == 0 ? UPDATED_CODE : ERROR_CODE;
#elif defined(__linux__)
    return pclose(pipe) == 0 ? UPDATED_CODE : ERROR_CODE;
#endif
}

int SystemInterface::linkDll(ProjectInfo *buildInfo, Mode mode, std::string path, std::string *output)
{
    std::string oFiles = "";
    for (const auto &tu : buildInfo->translationUnits)
        oFiles += "\"" + tu.oFilePath + "\" ";

    std::string libStr = "";
    for (const auto &lib : buildInfo->libs)
        libStr += "\"" + lib + "\" ";

    char buffer[1024];
#if defined(_WIN32)
    HANDLE out;
    PROCESS_INFORMATION pi;

    std::string prog = msvcInfo.compilerPath + "\\link.exe";
    std::string args = std::string(mode.config == Config::DEBUG ? " /DEBUG:FASTLINK " : " ") + "/DLL /out:\"" + path + "\" " + oFiles + buildInfo->linkerFlags + libStr + msvcInfo.systemLibPaths;

    FILE *pipe = win_popen(prog.c_str(), args.c_str(), &out, &pi);
#elif defined(__linux__)
    FILE *pipe = popen(("g++ --shared -march=x86-64 " + std::string(mode.arch == Arch::X86 ? "-m32 " : "-m64 ") + "-o \"" + path + "\" " + oFiles + "-Wl,--start-group " + buildInfo->linkerFlags + libStr + " -Wl,--end-group 2>&1").c_str(), "r");
#endif
    if (!pipe)
    {
        return ERROR_CODE;
    }

#if defined(_WIN32)
    for (int i = 0; i < 3; i++)
        fgets(buffer, 1024, pipe);
#endif

    while (fgets(buffer, 1024, pipe))
        *output += buffer;
#if defined(_WIN32)
    return win_pclose(&out, &pi) == 0 ? UPDATED_CODE : ERROR_CODE;
#elif defined(__linux__)
    return pclose(pipe) == 0 ? UPDATED_CODE : ERROR_CODE;
#endif
}

int SystemInterface::executeProgram(const char *prog, const char *args)
{
    char buffer[1024];
#if defined(_WIN32)
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFOA));

    char *argsBuffer;

    if (args == NULL)
    {
        argsBuffer = new char[strlen(prog) + 2];
        sprintf(argsBuffer, "\"%s\"", prog);
    }
    else
    {
        argsBuffer = new char[strlen(prog) + strlen(args) + 4];
        sprintf(argsBuffer, "\"%s\" %s", prog, args);
    }

    int bSuccess = CreateProcessA(prog,
                                  argsBuffer, // command line
                                  NULL,       // process security attributes
                                  NULL,       // primary thread security attributes
                                  TRUE,       // handles are inherited
                                  0,          // creation flags
                                  NULL,       // use parent's environment
                                  NULL,       // use parent's current directory
                                  &si,        // STARTUPINFO pointer
                                  &pi);       // receives PROCESS_INFORMATION

    delete[] argsBuffer;

    if (bSuccess)
    {
        // Wait for the process to exit
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Process has exited - check its exit code
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        // At this point exitCode is set to the process' exit code

        // Handles must be closed when they are no longer needed
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        return exitCode;
    }
    else
    {
        error("Could not run \'%s\'", prog);
    }
#elif defined(__linux__)
    return system(("\"" + std::string(prog) + "\"" + std::string(args == NULL ? "" : " " + std::string(args))).c_str());
#endif
}
/**/