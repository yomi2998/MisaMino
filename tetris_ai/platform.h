#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <thread>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace plat {

inline void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline double now_seconds() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

#ifdef _WIN32

typedef HMODULE DylibHandle;

inline DylibHandle dylib_open_raw(const std::string& path) {
    return ::LoadLibraryA(path.c_str());
}

inline void* dylib_sym(DylibHandle h, const char* name) {
    return (void*)::GetProcAddress(h, name);
}

inline const char* dylib_default_suffix() { return ".dll"; }

#else

typedef void* DylibHandle;

inline DylibHandle dylib_open_raw(const std::string& path) {
    return ::dlopen(path.c_str(), RTLD_NOW);
}

inline void* dylib_sym(DylibHandle h, const char* name) {
    return ::dlsym(h, name);
}

inline const char* dylib_default_suffix() { return ".so"; }

#endif

inline std::string file_suffix(std::string s) {
    size_t dot = s.rfind('.');
    if (dot == std::string::npos) return std::string();
    std::string ext = s.substr(dot);
    for (size_t i = 0; i < ext.size(); ++i) {
        if (ext[i] >= 'A' && ext[i] <= 'Z') ext[i] = (char)(ext[i] - 'A' + 'a');
    }
    return ext;
}

inline DylibHandle dylib_open(const std::string& name) {
    std::string native = name;
    std::string ext = file_suffix(native);
    if (ext != dylib_default_suffix()) {
        std::string rewritten = native.substr(0, native.size() - ext.size()) + dylib_default_suffix();
        std::error_code ec;
        if (std::filesystem::exists(rewritten, ec)) native = rewritten;
    }
#ifndef _WIN32
    if (native.find('/') == std::string::npos) native = "./" + native;
#endif
    return dylib_open_raw(native);
}

}
