#include <iostream>

#include <string>
#include <unistd.h>

#include "netfx/coreclr_delegates.h"
#include "netfx/hostfxr.h"
#include "netfx/nethost.h"

#if __APPLE__
#include <mach-o/dyld.h>
#elif __linux__

#include <dlfcn.h>

#endif

static void HOSTFXR_CALLTYPE ErrorWriter(const char_t *message) {
    fprintf(stderr, "%s\n", message);
}

std::string GetExePath() {
#ifndef NDEBUG
    if (getenv("TEST_EXE_PATH") != nullptr) {
        return getenv("TEST_EXE_PATH");
    }
#endif

    char exePath[2048];
#if __linux
    readlink("/proc/self/exe", exePath, 2048);
#elif __APPLE__
    _NSGetExecutablePath(exePath, 2048);
#endif
    return exePath;
}


std::string GetDirectoryName(std::string filename) {
    auto lastPathSeparator = filename.rfind('/');
    if (std::string::npos != lastPathSeparator)
        return filename.substr(0, lastPathSeparator);
    return "";
}

int Error(std::string s) {
    std::cerr << s << std::endl;
    return 1;
}

#define GetProc(name) auto name = (name ## _fn)dlsym(hostLib, #name); if(name == nullptr) return Error("dlsym " # name " failed")

int main(int argc, const char **argv) {

    auto exePath = GetExePath();
    auto dirPath = GetDirectoryName(exePath);
    if (dirPath.length() == 0)
        return Error("Unable to extract directory path from " + exePath);

    auto runtimeConfigPath = exePath + ".runtimeconfig.json";
    auto depsJsonPath = exePath + ".deps.json";
    auto dllPath = exePath + ".dll";
#if __linux
    auto hostfxrPath = dirPath + "/libhostfxr.so";
#elif __APPLE__
    auto hostfxrPath = dirPath + "/libhostfxr.dylib";
#endif
    auto hostLib = dlopen(hostfxrPath.c_str(), RTLD_NOW);
    if (hostLib == nullptr)
        return Error("dlopen failed for " + hostfxrPath + " code: " + std::to_string(errno));

    GetProc(hostfxr_initialize_for_dotnet_command_line);
    GetProc(hostfxr_run_app);
    GetProc(hostfxr_get_runtime_delegate);
    GetProc(hostfxr_set_error_writer);

    hostfxr_set_error_writer(&ErrorWriter);


    const char_t *runtimeArgs[] = {
            "exec",
            "--runtimeconfig",
            runtimeConfigPath.c_str(),
            "--depsfile",
            depsJsonPath.c_str(),
            dllPath.c_str(),
    };
    auto runtimeArgCount = sizeof(runtimeArgs) / sizeof(runtimeArgs[0]);
    auto appArgs = new  const char_t*[6 + argc - 1];
    for (size_t c = 0; c < runtimeArgCount; c++)
        appArgs[c] = runtimeArgs[c];
    for(size_t c = 0; c < argc - 1; c++)
        appArgs[runtimeArgCount + c] = argv[c + 1];


    void *context = nullptr;

    if (hostfxr_initialize_for_dotnet_command_line(runtimeArgCount + argc - 1, appArgs, nullptr, &context))
        return Error("hostfxr_initialize_for_dotnet_command_line failed");

    return hostfxr_run_app(context);
}
