#pragma once
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

#ifndef NDEBUG
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <map>
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

#ifndef NDEBUG
class MatrixEditors {
private:
  std::map<std::string, glm::mat4*> m_editors;

public:
  void add(const std::string &name, glm::mat4 &matrix) {
    m_editors[name] = &matrix;
  }

  void draw() {
    for (auto const&[name, matrix_ptr] : m_editors) {
      if (!ImGui::CollapsingHeader(name.c_str())) {
        continue;
      }

      ImGui::PushID(name.c_str());

      float buffer[16];
      std::memcpy(buffer, glm::value_ptr(glm::transpose(*matrix_ptr)), sizeof(buffer));

      bool value_changed = false;
      for (int i = 0; i < 4; ++i) {
        if (ImGui::InputFloat4(("##Row" + std::to_string(i)).c_str(),
                               &buffer[i * 4], "%.3f")) {
          value_changed = true;
        }
      }

      if (value_changed) {
        *matrix_ptr = glm::transpose(glm::make_mat4(buffer));
      }

      if (ImGui::Button("Reset")) {
        *matrix_ptr = glm::mat4(1.0f);
      }

      ImGui::PopID();
    }
  }
};
#endif
