#pragma once
#include "opengl.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#ifndef NDEBUG
#include "components/TransformComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <string>
#endif

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

inline std::string
getExeDir()
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
  if (pos == std::string::npos)
    return ".";
  return exePath.substr(0, pos);
}

inline std::vector<unsigned char>
read_file_bytes(const std::string& relativePath)
{
  std::string fullPath = getExeDir() + "/" + relativePath;
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

inline std::string
read_file(const std::string& relativePath)
{
  std::string fullPath = getExeDir() + "/" + relativePath;
  std::ifstream file(fullPath, std::ios::binary);
  if (!file.is_open())
    throw std::runtime_error("Cannot open file: " + fullPath);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

static void APIENTRY
glDebugOutput(GLenum source,
              GLenum type,
              unsigned int id,
              GLenum severity,
              GLsizei length,
              const char* message,
              const void* userParam)
{
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

static void
init_gl_debug()
{
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
    glDebugMessageControl(GL_DONT_CARE,
                          GL_DONT_CARE,
                          GL_DEBUG_SEVERITY_NOTIFICATION,
                          0,
                          nullptr,
                          GL_FALSE);
    std::cout << "Successfully initialized GL_KHR_debug." << std::endl;
    return;
  }

  if (glfwExtensionSupported("GL_ARB_debug_output")) {
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageCallbackARB(glDebugOutput, nullptr);
    std::cout << "Successfully initialized GL_ARB_debug_output." << std::endl;
    return;
  }

  std::cout << "Neither KHR_debug nor ARB_debug_output are supported."
            << std::endl;
}

#ifndef NDEBUG
inline void
draw_transform_component_editor(TransformComponent& component,
                                const char* label)
{
  if (!ImGui::CollapsingHeader(label)) {
    return;
  }

  glm::vec3 position = component.get_position();
  if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f)) {
    component.set_position(position);
  }

  glm::quat orientation = component.get_rotation();
  glm::vec3 euler_angles_rad = glm::eulerAngles(orientation);
  glm::vec3 euler_angles_deg = glm::degrees(euler_angles_rad);

  if (ImGui::DragFloat3("Rotation (Degrees)",
                        glm::value_ptr(euler_angles_deg))) {
    glm::vec3 new_euler_rad = glm::radians(euler_angles_deg);
    component.set_rotation(glm::quat(new_euler_rad));
  }

  glm::vec3 current_scale = component.get_scale();
  if (ImGui::DragFloat3("Scale", glm::value_ptr(current_scale), 0.05f)) {
    component.set_scale(current_scale);
  }

  ImGui::Separator();

  if (ImGui::TreeNode("Actions")) {
    static glm::vec3 translate_offset(0.0f);
    ImGui::InputFloat3("Translate Offset", glm::value_ptr(translate_offset));
    if (ImGui::Button("Translate")) {
      component.translate(translate_offset);
    }

    ImGui::Spacing();

    static glm::vec3 rotate_axis(0.0f, 1.0f, 0.0f);
    static float rotate_angle_deg = 45.0f;
    ImGui::InputFloat3("Rotation Axis", glm::value_ptr(rotate_axis));
    ImGui::SliderFloat("Rotation Angle", &rotate_angle_deg, -180.0f, 180.0f);
    if (ImGui::Button("Rotate")) {
      if (glm::length(rotate_axis) > 0.001f) {
        component.rotate(rotate_axis, glm::radians(rotate_angle_deg));
      }
    }

    ImGui::Spacing();

    static glm::vec3 scale_factors(1.0f);
    ImGui::InputFloat3("Scale Factors", glm::value_ptr(scale_factors));
    if (ImGui::Button("Scale")) {
      component.scale(scale_factors);
    }

    ImGui::TreePop();
  }
}
#endif

template<typename T, typename F>
auto
capture_weak(std::shared_ptr<T> obj, F&& f)
{
  static_assert(std::is_base_of_v<std::enable_shared_from_this<T>, T>,
                "T must inherit from enable_shared_from_this<T>");
  auto weak_obj = obj->weak_from_this();
  return [weak_obj, f = std::forward<F>(f)] {
    if (auto self = weak_obj.lock())
      f(self);
  };
}
