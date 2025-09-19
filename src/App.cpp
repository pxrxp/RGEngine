#include "App.h"
#include "definitions.h"
#include "Window.h"
#include "Camera.h"
#include "opengl.h"
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

#ifndef NDEBUG
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

float App::get_delta_time() const {
  return this->delta_time;
}

void App::init() {
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

  this->window.dimensions.x = video_mode->width / 2;
  this->window.dimensions.y = video_mode->height / 2;
  this->window.title = "RivalGrounds";

  this->window.raw_window =
      glfwCreateWindow(this->window.dimensions.x, this->window.dimensions.y,
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
Camera &App::get_camera() { return this->camera; }

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

void App::update_window_size() {
  glfwGetWindowSize(this->window.raw_window, &this->window.dimensions.x,
                    &this->window.dimensions.y);
}

void App::update_mouse_position() {
  this->window.old_mouse = this->window.mouse;

  double mousex, mousey;
  glfwGetCursorPos(this->window.raw_window, &mousex,
                   &mousey);
  this->window.mouse = this->window.ndc({mousex, mousey});
}

void App::update_delta_time() {
  this->current_frame_time = glfwGetTime();
  this->delta_time = static_cast<float> (this->current_frame_time - this->last_frame_time);
  this->last_frame_time = this->current_frame_time;
}

void App::update_camera() {
  auto delta_mouse = this->get_window().get_delta_mouse();

  this->camera.yaw += delta_mouse.x * this->camera.sensitivity * this->delta_time;
  this->camera.pitch += delta_mouse.y * this->camera.sensitivity * this->delta_time;

  this->camera.pitch = glm::clamp(this->camera.pitch, -89.f, +89.f);

  glm::mat4 camera_transform(1.f);
  camera_transform = glm::rotate(camera_transform, this->camera.yaw, Y_AXIS);
  camera_transform = glm::rotate(camera_transform, this->camera.pitch, X_AXIS);
  camera_transform = glm::translate(camera_transform, this->camera.pos);

  this->camera.view = glm::inverse(camera_transform);
}

void App::update() {
  if (!this->window.raw_window)
    return;

  glfwPollEvents();

  this->update_window_size();
  this->update_mouse_position();
  this->update_camera();
  this->update_delta_time();
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
