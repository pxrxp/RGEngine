#include "App.h"
#include "Camera.h"
#include "Window.h"
#include "util.h"
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

#include "debug.h"

#ifndef NDEBUG
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

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

void App::update_delta_time() {
  m_current_frame_time = glfwGetTime();
  m_delta_time =
      static_cast<float>(m_current_frame_time - m_last_frame_time);
  m_last_frame_time = m_current_frame_time;
}

void App::update() {
  if (!m_window.m_raw_window)
    return;

  glfwPollEvents();

  if (m_window.get_cursor_mode() == GLFW_CURSOR_DISABLED)
    m_camera.update_lazy(*this);

  m_window.m_mouse_accumulator = {0., 0.};

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
