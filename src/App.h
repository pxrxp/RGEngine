#pragma once
#include "opengl.h"
#include <string>

class App;

class Window {
private:
  int width;
  int height;
  GLFWwindow *raw_window = nullptr;
  std::string title;

public:
  int get_width() const { return width; }
  int get_height() const { return height; }
  float get_aspect_ratio() const { return (float) width / height; }
  void swap_buffers() { glfwSwapBuffers(this->raw_window); }
  void clear(GLfloat color_r, GLfloat color_g, GLfloat color_b, GLfloat color_a, GLbitfield mask) {
    glClearColor(color_r, color_g, color_b, color_a);
    glClear(mask);
  }

  friend class App;
};

class App {
private:
  Window window;
  bool imgui_initialized = false;

public:
  ~App();
  bool is_running() const;
  void init_window();
  Window &get_window();
  void update_window();
  void init_debug_gui();
  void render_debug_gui();
};
