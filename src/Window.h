#pragma once
#include "opengl.h"
#include <glm/glm.hpp>
#include <string>

class App;

class Window {
private:
  glm::ivec2 dimensions = {0,0};
  glm::dvec2 old_mouse = {0.,0.};
  glm::dvec2 mouse = {0.,0.};
  GLFWwindow *raw_window = nullptr;
  std::string title;

public:
  int get_width() const { return dimensions.x; }
  int get_height() const { return dimensions.y; }
  float get_aspect_ratio() const { return (float) dimensions.x / dimensions.y; }

  glm::dvec2 ndc(glm::dvec2 screen_coords) const;
  glm::dvec2 get_delta_mouse() const;
  glm::dvec2 get_mouse_pos() const;
  void swap_buffers();
  void clear(GLfloat color_r, GLfloat color_g, GLfloat color_b, GLfloat color_a,
             GLbitfield mask);

  friend class App;
};
