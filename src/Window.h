#pragma once
#include "opengl.h"
#include <glm/glm.hpp>
#include <string>

class App;

class Window {
private:
  glm::ivec2 m_dimensions = {0,0};
  glm::dvec2 m_old_mouse = {0.,0.};
  glm::dvec2 m_mouse = {0.,0.};
  GLFWwindow *m_raw_window = nullptr;
  std::string m_title;

public:
  int get_width() const { return m_dimensions.x; }
  int get_height() const { return m_dimensions.y; }
  float get_aspect_ratio() const { return (float) m_dimensions.x / m_dimensions.y; }

  // glm::dvec2 ndc(glm::dvec2 screen_coords) const;
  glm::dvec2 get_delta_mouse() const;
  glm::dvec2 get_mouse_pos() const;
  void set_cursor_mode(int mode);
  int get_cursor_mode();
  void swap_buffers();
  void clear(GLfloat color_r, GLfloat color_g, GLfloat color_b, GLfloat color_a,
             GLbitfield mask);

  friend class App;
};
