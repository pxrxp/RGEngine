#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class App;

class Window {
private:
  int width;
  int height;
  GLFWwindow *raw_window;
  std::string title;

public:
  int get_width() { return width; }
  int get_height() { return height; }
  void swap_buffers() { glfwSwapBuffers(this->raw_window); }

  friend class App;
};

class App {
private:
  Window window;

public:
  bool is_running();
  void init_window();
  void terminate_window();
  Window &get_window();
  void update();
};
