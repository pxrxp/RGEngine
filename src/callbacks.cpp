#include "App.h"
#include "opengl.h"

#include "debug.h"
#include <GLFW/glfw3.h>

void App::register_callbacks() {
  glfwSetFramebufferSizeCallback(m_window.m_raw_window,
                                 framebuffer_size_callback);
  glfwSetKeyCallback(m_window.m_raw_window, key_callback);
}

void App::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  glViewport(0, 0, width, height);
  app->m_window.m_dimensions = {width, height};
  app->m_camera.update_projection_matrix(app->m_window.get_aspect_ratio());
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
