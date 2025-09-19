#pragma once
#include <fstream>
#include <sstream>
#include <algorithm>
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

template <typename T>
inline T clamp_map(T x, std::pair<T, T> from_range, std::pair<T, T> to_range) {
    T clamped_x = std::clamp(x, from_range.first, from_range.second);
    
    T from_span = from_range.second - from_range.first;
    T to_span = to_range.second - to_range.first;

    if (from_span == 0) {
        return to_range.first;
    }

    double normalized_value = static_cast<double>(clamped_x - from_range.first) / 
                              static_cast<double>(from_span);
    
    return static_cast<T>(to_range.first + normalized_value * to_span);
}

#ifndef NDEBUG
struct MatrixEditor {
    glm::mat4& mat;
    float buf[16];
    bool dirty = false;
    bool initialized = false;

    MatrixEditor(glm::mat4& m) : mat(m) {}

    bool Draw(const char* label) {
        if (!initialized) {
            std::memcpy(buf, glm::value_ptr(glm::transpose(mat)), sizeof(buf));
            initialized = true;
        }

        ImGui::PushID(label);
        bool changed = false;

        for (int row = 0; row < 4; ++row) {
            char rowLabel[16];
            snprintf(rowLabel, sizeof(rowLabel), "Row %d", row);
            changed |= ImGui::InputFloat4(rowLabel, &buf[row * 4]);
        }

        ImGui::Separator();

        if (ImGui::Button("Apply")) {
            mat = glm::transpose(glm::make_mat4(buf));
            changed = true;
            dirty = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            mat = glm::mat4(1.f);
            std::memcpy(buf, glm::value_ptr(glm::transpose(mat)), sizeof(buf));
            changed = true;
            dirty = false;
        }

        ImGui::PopID();
        dirty |= changed;
        return changed;
    }
};
#endif
