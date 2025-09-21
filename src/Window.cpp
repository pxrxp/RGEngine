#include "Window.h"
#include "opengl.h"
#include "util.h"
#include <glm/glm.hpp>

glm::dvec2 Window::get_delta_mouse() const {
  return m_mouse - m_old_mouse;
}

glm::dvec2 Window::get_mouse_pos() const {
  return m_mouse;
}

void Window::set_cursor_mode(int mode) {
  glfwSetInputMode(m_raw_window, GLFW_CURSOR, mode);
}

int Window::get_cursor_mode() {
  return glfwGetInputMode(m_raw_window, GLFW_CURSOR);
}

// glm::dvec2 Window::ndc(glm::dvec2 screen_coords) const {
//   double x_ndc = clamp_map(screen_coords.x, {0, dimensions.x}, {-1., 1.});
//   double y_ndc = clamp_map(screen_coords.y, {0, dimensions.y}, {1., -1.});
//   return glm::vec2(x_ndc, y_ndc);
// }

void Window::swap_buffers() { glfwSwapBuffers(m_raw_window); }
void Window::clear(GLfloat color_r, GLfloat color_g, GLfloat color_b,
                   GLfloat color_a, GLbitfield mask) {
  glClearColor(color_r, color_g, color_b, color_a);
  glClear(mask);
}
