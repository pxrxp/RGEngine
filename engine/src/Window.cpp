#include <engine/Window.h>
#include <engine/util/log.h>
#include <engine/util/opengl.h>
#include <iostream>

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

bool
Window::is_mouse_button_pressed(int button) const
{
  if (!m_raw_window)
    return false;

  return glfwGetMouseButton(m_raw_window, button) == GLFW_PRESS;
}

glm::dvec2
Window::get_delta_mouse() const
{
  return m_mouse_accumulator;
}

void
Window::set_cursor_mode(int mode)
{
  if (!m_raw_window)
    return;

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
  if (!m_raw_window)
    return;

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

Window::~Window()
{
  if (m_raw_window) {
#ifndef NDEBUG
    Util::log_out() << "Destroying window" << std::endl;
#endif

    glfwDestroyWindow(m_raw_window);
  }

#ifndef NDEBUG
  Util::log_out() << "Terminating windowing library" << std::endl;
#endif

  glfwTerminate();
}
