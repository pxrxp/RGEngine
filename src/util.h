#pragma once
#include "opengl.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

inline std::vector<unsigned char>
read_file_bytes(const std::string &relativePath) {
  std::string fullPath = getExeDir() + "/" + relativePath;
  std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    throw std::runtime_error("Cannot open file: " + fullPath);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<unsigned char> buffer(size);
  if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    throw std::runtime_error("Cannot read file: " + fullPath);
  return buffer;
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

static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
                            GLenum severity, GLsizei length,
                            const char *message, const void *userParam) {
  // Ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    return;

  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    return;

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " << message << std::endl;

  switch (source) {
  case GL_DEBUG_SOURCE_API:
    std::cout << "Source: API";
    break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    std::cout << "Source: Window System";
    break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    std::cout << "Source: Shader Compiler";
    break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    std::cout << "Source: Third Party";
    break;
  case GL_DEBUG_SOURCE_APPLICATION:
    std::cout << "Source: Application";
    break;
  case GL_DEBUG_SOURCE_OTHER:
    std::cout << "Source: Other";
    break;
  }
  std::cout << std::endl;

  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    std::cout << "Type: Error";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    std::cout << "Type: Deprecated Behaviour";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    std::cout << "Type: Undefined Behaviour";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    std::cout << "Type: Portability";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    std::cout << "Type: Performance";
    break;
  case GL_DEBUG_TYPE_MARKER:
    std::cout << "Type: Marker";
    break;
  case GL_DEBUG_TYPE_PUSH_GROUP:
    std::cout << "Type: Push Group";
    break;
  case GL_DEBUG_TYPE_POP_GROUP:
    std::cout << "Type: Pop Group";
    break;
  case GL_DEBUG_TYPE_OTHER:
    std::cout << "Type: Other";
    break;
  }
  std::cout << std::endl;

  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    std::cout << "Severity: high";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    std::cout << "Severity: medium";
    break;
  case GL_DEBUG_SEVERITY_LOW:
    std::cout << "Severity: low";
    break;
  }
  std::cout << std::endl;
  std::cout << std::endl;
}

static void init_gl_debug() {
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (!(flags & GL_CONTEXT_FLAG_DEBUG_BIT)) {
        std::cout << "Debug context not available." << std::endl;
        return;
    }

    if (glfwExtensionSupported("GL_KHR_debug")) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        std::cout << "Successfully initialized GL_KHR_debug." << std::endl;
        return;
    }

    if (glfwExtensionSupported("GL_ARB_debug_output")) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(glDebugOutput, nullptr);
        std::cout << "Successfully initialized GL_ARB_debug_output." << std::endl;
        return;
    }

    std::cout << "Neither KHR_debug nor ARB_debug_output are supported." << std::endl;
}

struct GLDebugScope {
    GLDebugScope(const char* name) { glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name); }
    ~GLDebugScope() { glPopDebugGroup(); }
};
