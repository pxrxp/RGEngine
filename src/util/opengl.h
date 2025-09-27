#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

inline bool is_opengl_context_active() {
  return glfwGetCurrentContext() != NULL;
}
