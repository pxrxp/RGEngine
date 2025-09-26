#include "filesystem.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

namespace Util {

std::string
get_exe_dir()
{
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
  return (pos == std::string::npos) ? "." : exePath.substr(0, pos);
}

std::vector<unsigned char>
read_file_bytes(const std::string& relative_path)
{
  std::string fullPath = get_exe_dir() + "/" + relative_path;
  std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    throw std::runtime_error("Cannot open file: " + fullPath);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<unsigned char> buffer(size);
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
    throw std::runtime_error("Cannot read file: " + fullPath);
  return buffer;
}

std::string
read_file(const std::string& relative_path)
{
  std::string fullPath = get_exe_dir() + "/" + relative_path;
  std::ifstream file(fullPath, std::ios::binary);
  if (!file.is_open())
    throw std::runtime_error("Cannot open file: " + fullPath);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

}
