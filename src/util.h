#pragma once
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

inline std::string getExeDir() {
  std::string exePath;
#if defined(_WIN32)
  char path[MAX_PATH];
  GetModuleFileNameA(NULL, path, MAX_PATH);
  exePath = path;
#elif defined(__APPLE__)
  char path[PATH_MAX];
  uint32_t size = sizeof(path);
  _NSGetExecutablePath(path, &size);
  exePath = path;
#elif defined(__linux__)
  char path[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
  if (count <= 0)
    throw std::runtime_error("Cannot get executable path");
  exePath.assign(path, count);
#else
#error Unsupported platform
#endif
  size_t pos = exePath.find_last_of("\\/");
  if (pos == std::string::npos)
    return ".";
  return exePath.substr(0, pos);
}

inline std::string read_file(const std::string &relativePath) {
  std::string fullPath = getExeDir() + "/" + relativePath;
  std::ifstream file(fullPath, std::ios::binary);
  if (!file.is_open())
    throw std::runtime_error("Cannot open file: " + fullPath);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}
