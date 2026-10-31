#include "App.h"
#include "ShaderProgram.h"
#include "util/gl_debug.h"
#include <algorithm>
#include <iostream>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace {
constexpr int SCENE_SAMPLE_COUNT = 4;
}

void
App::init()
{
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  auto video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

  m_window.m_dimensions.x = video_mode->width / 2;
  m_window.m_dimensions.y = video_mode->height / 2;
  m_window.m_title = "RivalGrounds";

  m_window.m_raw_window = glfwCreateWindow(m_window.m_dimensions.x,
                                           m_window.m_dimensions.y,
                                           m_window.m_title.c_str(),
                                           nullptr,
                                           nullptr);

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

  Util::init_gl_debug();

  int width, height;
  glfwGetFramebufferSize(m_window.m_raw_window, &width, &height);
  glViewport(0, 0, width, height);

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(m_window.m_raw_window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
  m_imgui_initialized = true;

  material_repo.clear_all_slots();
}

bool
App::is_running() const
{
  if (m_window.m_raw_window)
    return !glfwWindowShouldClose(m_window.m_raw_window);
  return false;
}

Window&
App::get_window()
{
  return m_window;
}
Camera&
App::get_camera()
{
  return m_camera;
}
Renderer&
App::get_renderer()
{
  return m_renderer;
}
float
App::get_delta_time() const
{
  return m_delta_time;
}

App::~App()
{
  if (m_imgui_initialized) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }
}

void
App::update_delta_time()
{
  m_current_frame_time = glfwGetTime();
  m_delta_time = static_cast<float>(m_current_frame_time - m_last_frame_time);
  m_last_frame_time = m_current_frame_time;
}

void
App::update()
{
  if (!m_window.m_raw_window)
    return;

  glfwPollEvents();

  if (m_window.get_cursor_mode() == GLFW_CURSOR_DISABLED)
    m_camera.update_lazy(*this);

  m_window.m_mouse_accumulator = { 0., 0. };

  update_delta_time();
}

void
App::cleanup()
{
  m_renderer.clear();
  material_repo.shader_program_repo.shader_repo.clear();
  material_repo.shader_program_repo.clear();
  material_repo.tex_repo.clear();
  material_repo.clear();
  mesh_repo.clear();
}
void
App::init_debug_gui()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void
App::render_debug_gui()
{
  // Crosshair
  if (m_window.get_cursor_mode() == GLFW_CURSOR_DISABLED) {
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    const ImGuiIO& io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    draw_list->AddCircleFilled(center, 2.0f, IM_COL32(255, 255, 255, 230));
    draw_list->AddLine(ImVec2(center.x - 9, center.y),
                       ImVec2(center.x - 4, center.y),
                       IM_COL32(255, 255, 255, 180),
                       1.5f);
    draw_list->AddLine(ImVec2(center.x + 4, center.y),
                       ImVec2(center.x + 9, center.y),
                       IM_COL32(255, 255, 255, 180),
                       1.5f);
    draw_list->AddLine(ImVec2(center.x, center.y - 9),
                       ImVec2(center.x, center.y - 4),
                       IM_COL32(255, 255, 255, 180),
                       1.5f);
    draw_list->AddLine(ImVec2(center.x, center.y + 4),
                       ImVec2(center.x, center.y + 9),
                       IM_COL32(255, 255, 255, 180),
                       1.5f);
  }

  glDisable(GL_DEPTH_TEST);
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glEnable(GL_DEPTH_TEST);
}
