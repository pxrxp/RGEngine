#pragma once
#include "opengl.h"
#include <glm/glm.hpp>
#include <string>

class App;

class Window
{
private:
  glm::ivec2 m_dimensions = { 0, 0 };
  glm::dvec2 m_mouse_accumulator = { 0., 0. };
  glm::dvec2 m_mouse = { 0., 0. };
  GLFWwindow* m_raw_window = nullptr;
  std::string m_title;

public:
  glm::ivec2 get_dimensions() const;
  int get_width() const;
  int get_height() const;
  float get_aspect_ratio() const;

  bool is_key_pressed(int key) const;
  glm::dvec2 get_delta_mouse() const;
  glm::dvec2 get_mouse_pos() const;
  void set_cursor_mode(int mode);
  int get_cursor_mode();
  void swap_buffers();
  void clear(GLfloat color_r,
             GLfloat color_g,
             GLfloat color_b,
             GLfloat color_a,
             GLbitfield mask);

  friend class App;
};
