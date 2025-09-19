#include "Window.h"
#include "util.h"
#include <glm/glm.hpp>

glm::dvec2 Window::get_delta_mouse() const {
  return this->mouse - this->old_mouse;
}

glm::dvec2 Window::get_mouse_pos() const {
  return this->mouse;
}

glm::dvec2 Window::ndc(glm::dvec2 screen_coords) const {
  double x_ndc = clamp_map(screen_coords.x, {0, this->dimensions.x}, {-1., 1.});
  double y_ndc = clamp_map(screen_coords.y, {0, this->dimensions.y}, {1., -1.});
  return glm::vec2(x_ndc, y_ndc);
}

void Window::swap_buffers() { glfwSwapBuffers(this->raw_window); }
void Window::clear(GLfloat color_r, GLfloat color_g, GLfloat color_b,
                   GLfloat color_a, GLbitfield mask) {
  glClearColor(color_r, color_g, color_b, color_a);
  glClear(mask);
}
