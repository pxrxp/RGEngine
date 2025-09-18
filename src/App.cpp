#include "App.h"
#include "opengl.h"
#include "util.h"
#include <iostream>
#include <utility>

#ifndef NDEBUG
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

std::pair<double, double> Window::get_mouse_pos_ndc() const {
  double x_ndc = clamp_map(mouse_x, 0, width, -1., 1.);
  double y_ndc = -clamp_map(mouse_y, 0, height, -1., 1.);
  return std::make_pair(x_ndc, y_ndc);
}

void Window::swap_buffers() { glfwSwapBuffers(this->raw_window); }
void Window::clear(GLfloat color_r, GLfloat color_g, GLfloat color_b,
                   GLfloat color_a, GLbitfield mask) {
  glClearColor(color_r, color_g, color_b, color_a);
  glClear(mask);
}

void App::init_window() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
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
  glfwSetFramebufferSizeCallback(this->window.raw_window,
                                 framebuffer_size_callback);
  glfwSwapInterval(1);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    exit(EXIT_FAILURE);
  }

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef NDEBUG
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(this->window.raw_window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
  this->imgui_initialized = true;
#endif
}

bool App::is_running() const {
  if (this->window.raw_window)
    return !glfwWindowShouldClose(this->window.raw_window);
  return false;
}

Window &App::get_window() { return this->window; }

App::~App() {
#ifndef NDEBUG
  if (imgui_initialized) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }
#endif

  if (this->window.raw_window)
    glfwDestroyWindow(this->window.raw_window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}

void App::update_window() {
  if (!this->window.raw_window)
    return;
  glfwPollEvents();
  glfwGetWindowSize(this->window.raw_window, &this->window.width,
                    &this->window.height);
  glfwGetCursorPos(this->window.raw_window, &this->window.mouse_x,
                   &this->window.mouse_y);
}

void App::init_debug_gui() {
#ifndef NDEBUG
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
#endif
}

void App::render_debug_gui() {
#ifndef NDEBUG
  glDisable(GL_DEPTH_TEST);
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glEnable(GL_DEPTH_TEST);
#endif
}
