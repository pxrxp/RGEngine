#include "Window.h"
#include "util/opengl.h"

glm::ivec2
Window::get_dimensions() const
{
  return m_dimensions;
}
int
Window::get_width() const
{
  return m_dimensions.x;
}
int
Window::get_height() const
{
  return m_dimensions.y;
}
float
Window::get_aspect_ratio() const
{
  return (float)m_dimensions.x / m_dimensions.y;
}

bool
Window::is_key_pressed(int key) const
{
  if (!m_raw_window)
    return false;

  return glfwGetKey(m_raw_window, key) == GLFW_PRESS;
}

glm::dvec2
Window::get_delta_mouse() const
{
  return m_mouse_accumulator;
}

glm::dvec2
Window::get_mouse_pos() const
{
  return m_mouse;
}

void
Window::set_cursor_mode(int mode)
{
  glfwSetInputMode(m_raw_window, GLFW_CURSOR, mode);
}

int
Window::get_cursor_mode()
{
  return glfwGetInputMode(m_raw_window, GLFW_CURSOR);
}

void
Window::swap_buffers()
{
  glfwSwapBuffers(m_raw_window);
}

void
Window::clear(GLfloat color_r,
              GLfloat color_g,
              GLfloat color_b,
              GLfloat color_a,
              GLbitfield mask)
{
  glClearColor(color_r, color_g, color_b, color_a);
  glClear(mask);
}
