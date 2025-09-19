#include "App.h"
#include "Camera.h"
#include "Window.h"
#include "definitions.h"
#include "opengl.h"
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

#ifndef NDEBUG
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_ESCAPE)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  if (key == GLFW_KEY_TAB)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void cursor_enter_callback(GLFWwindow *window, int entered) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));
  if (!app)
    return;

  app->set_cursor_inside(static_cast<bool>(entered));
}

float App::get_delta_time() const { return this->delta_time; }
bool App::is_cursor_inside() const { return this->cursor_inside; }
void App::set_cursor_inside(bool inside) { this->cursor_inside = inside; std::cout << "imin\n";}

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
  glfwSetWindowUserPointer(this->window.raw_window, this);
  glfwSetInputMode(this->window.raw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetKeyCallback(this->window.raw_window, key_callback);

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
  glfwGetCursorPos(this->window.raw_window, &mousex, &mousey);
  this->window.mouse = {mousex, mousey};
}

void App::update_delta_time() {
  this->current_frame_time = glfwGetTime();
  this->delta_time =
      static_cast<float>(this->current_frame_time - this->last_frame_time);
  this->last_frame_time = this->current_frame_time;
}
int i = 0;

void App::update_camera() {
  if (this->window.get_cursor_mode() != GLFW_CURSOR_DISABLED)
    return;

  auto delta_mouse = this->window.get_delta_mouse();
  if (delta_mouse.x == 0. && delta_mouse.y == 0.)
    return;

  if (this->window.dimensions.y <= 0)
    return;

  float px_to_rad = this->camera.fovy_rad / this->window.dimensions.y;

  this->camera.yaw_rad +=
      delta_mouse.x * px_to_rad * this->camera.sensitivity * this->delta_time;
  this->camera.pitch_rad +=
      delta_mouse.y * px_to_rad * this->camera.sensitivity * this->delta_time;

  const float max_pitch = glm::radians(89.0f);
  this->camera.pitch_rad =
      glm::clamp(this->camera.pitch_rad, -max_pitch, max_pitch);

  glm::vec3 forward = glm::normalize(
      glm::vec3(cos(this->camera.yaw_rad) * cos(this->camera.pitch_rad),
                sin(this->camera.pitch_rad),
                sin(this->camera.yaw_rad) * cos(this->camera.pitch_rad)));
  glm::vec3 right = glm::normalize(glm::cross(forward, Y_AXIS));
  glm::vec3 up = glm::normalize(glm::cross(right, forward));
  glm::vec3 target = this->camera.pos + forward;

  this->camera.view = glm::lookAt(this->camera.pos, target, up);

  if (i <= 4) {
    std::cout << this->camera.view << std::endl;
    i++;
  }
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
