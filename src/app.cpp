#include "app.h"
#include <cstdlib>
#include <iostream>

void App::init_window() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto primary_monitor = glfwGetPrimaryMonitor();
  auto video_mode = glfwGetVideoMode(primary_monitor);

  this->window.width = video_mode->width / 2;
  this->window.height = video_mode->height / 2;
  this->window.title = "RivalGrounds";

  this->window.raw_window =
      glfwCreateWindow(this->window.width, this->window.height,
                       this->window.title.c_str(), nullptr, nullptr);

  if (!this->window.raw_window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(this->window.raw_window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    exit(EXIT_FAILURE);
  }
}

bool App::is_running() {
  return !glfwWindowShouldClose(this->window.raw_window);
}

Window &App::get_window() { return this->window; }

void App::terminate_window() {
  glfwDestroyWindow(this->window.raw_window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

void App::update() {
  glfwGetWindowSize(this->window.raw_window, &this->window.width,
                    &this->window.height);
}
