#pragma once
#include "opengl.h"
#include <utility>
#include <string>

class App;

class Window {
private:
  int width;
  int height;
  double mouse_x, mouse_y;
  GLFWwindow *raw_window = nullptr;
  std::string title;

public:
  int get_width() const { return width; }
  int get_height() const { return height; }
  float get_aspect_ratio() const { return (float)width / height; }

  std::pair<double, double> get_mouse_pos_ndc() const;
  void swap_buffers();
  void clear(GLfloat color_r, GLfloat color_g, GLfloat color_b, GLfloat color_a,
             GLbitfield mask);

  friend class App;
};
