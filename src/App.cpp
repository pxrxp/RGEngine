#include "App.h"
#include "Camera.h"
#include "Window.h"
#include "definitions.h"
#include "util.h"
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

#ifndef NDEBUG
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

bool App::is_key_pressed(int key) const {
  if (!m_window.m_raw_window)
    return false;

  return glfwGetKey(m_window.m_raw_window, key) == GLFW_PRESS;
}

glm::vec3 App::get_camera_move_dir() const {
  glm::vec3 direction(0.f);

  bool go_forward =
      is_key_pressed(GLFW_KEY_W) || is_key_pressed(GLFW_KEY_UP);
  bool go_backward =
      is_key_pressed(GLFW_KEY_S) || is_key_pressed(GLFW_KEY_DOWN);
  bool go_left =
      is_key_pressed(GLFW_KEY_A) || is_key_pressed(GLFW_KEY_LEFT);
  bool go_right =
      is_key_pressed(GLFW_KEY_D) || is_key_pressed(GLFW_KEY_RIGHT);
  bool go_up = is_key_pressed(GLFW_KEY_Q);
  bool go_down = is_key_pressed(GLFW_KEY_E);
  bool go_fast = is_key_pressed(GLFW_KEY_LEFT_CONTROL) ||
                 is_key_pressed(GLFW_KEY_RIGHT_CONTROL);
  bool go_slow = is_key_pressed(GLFW_KEY_LEFT_SHIFT) ||
                 is_key_pressed(GLFW_KEY_RIGHT_SHIFT);

  if (go_forward)
    direction.z++;
  if (go_backward)
    direction.z--;
  if (go_left)
    direction.x--;
  if (go_right)
    direction.x++;
  if (go_up)
    direction.y++;
  if (go_down)
    direction.y--;
  if (go_fast)
    direction *= 2;
  if (go_slow)
    direction /= 2;

  return direction;
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
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  auto video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

  m_window.m_dimensions.x = video_mode->width / 2;
  m_window.m_dimensions.y = video_mode->height / 2;
  m_window.m_title = "RivalGrounds";

  m_window.m_raw_window =
      glfwCreateWindow(m_window.m_dimensions.x, m_window.m_dimensions.y,
                       m_window.m_title.c_str(), nullptr, nullptr);

  if (!m_window.m_raw_window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(m_window.m_raw_window);
  glfwSwapInterval(1);
  glfwSetWindowUserPointer(m_window.m_raw_window, this);

  m_window.set_cursor_mode(GLFW_CURSOR_DISABLED);
  register_callbacks();

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    exit(EXIT_FAILURE);
  }

  init_gl_debug();

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
  ImGui_ImplGlfw_InitForOpenGL(m_window.m_raw_window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
  m_imgui_initialized = true;
#endif
}

bool App::is_running() const {
  if (m_window.m_raw_window)
    return !glfwWindowShouldClose(m_window.m_raw_window);
  return false;
}

Window &App::get_window() { return m_window; }
Camera &App::get_camera() { return m_camera; }
Renderer &App::get_renderer() { return m_renderer; }
float App::get_delta_time() const { return m_delta_time; }

App::~App() {
#ifndef NDEBUG
  if (m_imgui_initialized) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }
#endif

  if (m_window.m_raw_window)
    glfwDestroyWindow(m_window.m_raw_window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}

void App::update_window_size() {
  glfwGetWindowSize(m_window.m_raw_window, &m_window.m_dimensions.x,
                    &m_window.m_dimensions.y);
}

void App::update_mouse_position() {
  m_window.m_old_mouse = m_window.m_mouse;

  double mousex, mousey;
  glfwGetCursorPos(m_window.m_raw_window, &mousex, &mousey);
  m_window.m_mouse = {mousex, mousey};
}

void App::update_delta_time() {
  m_current_frame_time = glfwGetTime();
  m_delta_time =
      static_cast<float>(m_current_frame_time - m_last_frame_time);
  m_last_frame_time = m_current_frame_time;
}
int i = 0;

void App::update_camera() {
  if (m_window.get_cursor_mode() != GLFW_CURSOR_DISABLED)
    return;

  if (m_window.m_dimensions.y <= 0)
    return;

  auto delta_mouse = m_window.get_delta_mouse();
  auto move_dir = get_camera_move_dir();

  bool mouse_inactive = delta_mouse.x == 0 && delta_mouse.y == 0;
  bool keyboard_inactive =
      move_dir.x == 0 && move_dir.y == 0 && move_dir.z == 0;

  if (mouse_inactive && keyboard_inactive)
    return;

  float px_to_rad = m_camera.m_fovy_rad / m_window.m_dimensions.y;

  m_camera.m_yaw_rad +=
      delta_mouse.x * px_to_rad * m_camera.m_sensitivity * m_delta_time;
  m_camera.m_pitch_rad +=
      -delta_mouse.y * px_to_rad * m_camera.m_sensitivity * m_delta_time;

  const float max_pitch = glm::radians(89.0f);
  m_camera.m_pitch_rad =
      glm::clamp(m_camera.m_pitch_rad, -max_pitch, max_pitch);

  glm::vec3 forward = glm::normalize(
      glm::vec3(cos(m_camera.m_yaw_rad) * cos(m_camera.m_pitch_rad),
                sin(m_camera.m_pitch_rad),
                sin(m_camera.m_yaw_rad) * cos(m_camera.m_pitch_rad)));
  glm::vec3 right = glm::normalize(glm::cross(forward, AXIS_Y));
  glm::vec3 up = glm::normalize(glm::cross(right, forward));

  m_camera.m_pos += m_camera.m_speed * move_dir.x * right;
  m_camera.m_pos += m_camera.m_speed * move_dir.y * up;
  m_camera.m_pos += m_camera.m_speed * move_dir.z * forward;

  glm::vec3 target = m_camera.m_pos + forward;
  m_camera.m_view = glm::lookAt(m_camera.m_pos, target, up);
}

void App::update() {
  if (!m_window.m_raw_window)
    return;

  glfwPollEvents();

  update_window_size();
  update_mouse_position();
  update_camera();
  update_delta_time();
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
