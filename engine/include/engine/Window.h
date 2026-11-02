#pragma once
#include <engine/util/definitions.h>
#include <glm/glm.hpp>
#include <string>

class App;
struct GLFWwindow;

class Window
{
private:
  glm::ivec2 m_dimensions = { 0, 0 };
  glm::dvec2 m_mouse_accumulator = { 0., 0. };
  glm::dvec2 m_mouse = { 0., 0. };
  bool m_first_mouse_update = true;
  GLFWwindow* m_raw_window = nullptr;
  std::string m_title;

public:
  int get_height() const;
  float get_aspect_ratio() const;

  bool is_key_pressed(int key) const;
  bool is_mouse_button_pressed(int button) const;
  glm::dvec2 get_delta_mouse() const;
  void set_cursor_mode(int mode);
  int get_cursor_mode();
  void swap_buffers();
  void clear(GLfloat color_r,
             GLfloat color_g,
             GLfloat color_b,
             GLfloat color_a,
             GLbitfield mask);

  ~Window();

  friend class App;
};
