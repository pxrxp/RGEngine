#include "App.h"
#include "opengl.h"

void App::register_callbacks() {
  glfwSetFramebufferSizeCallback(this->window.raw_window, framebuffer_size_callback);
  glfwSetKeyCallback(this->window.raw_window, key_callback);
}

void App::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void App::key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_ESCAPE)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  if (key == GLFW_KEY_TAB)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
