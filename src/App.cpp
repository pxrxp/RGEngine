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

  glfwWindowHint(GLFW_SAMPLES, SCENE_SAMPLE_COUNT);
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
  glEnable(GL_DEPTH_TEST);

  create_postprocess_program();
  create_scene_framebuffer(width, height);

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
  destroy_scene_render_resources();
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

bool
App::create_postprocess_program()
{
  m_postprocess_program = material_repo.shader_program_repo.load_program(
    { .program_name = "grayscale_postprocess",
      .shaders = { "postprocess.vert", "postprocess.frag" } });

  if (!m_postprocess_program || m_postprocess_program->get_id() == 0)
    return false;

  m_postprocess_program->set_uniform("u_scene", 0);
  m_postprocess_program->set_uniform("u_scene_mask", 1);
  glGenVertexArrays(1, &m_postprocess_vertex_array);
  return m_postprocess_vertex_array != 0;
}

bool
App::create_scene_framebuffer(int width, int height)
{
  if (width <= 0 || height <= 0 || !m_postprocess_program ||
      m_postprocess_vertex_array == 0)
    return false;

  if (m_scene_framebuffer_ready && width == m_scene_buffer_width &&
      height == m_scene_buffer_height)
    return true;

  GLint previous_draw_framebuffer = 0;
  GLint previous_read_framebuffer = 0;
  GLint previous_texture = 0;
  GLint previous_renderbuffer = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previous_draw_framebuffer);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_texture);
  glGetIntegerv(GL_RENDERBUFFER_BINDING, &previous_renderbuffer);

  auto delete_framebuffer_resources = [this]() {
    if (m_scene_framebuffer)
      glDeleteFramebuffers(1, &m_scene_framebuffer);
    if (m_scene_resolve_framebuffer)
      glDeleteFramebuffers(1, &m_scene_resolve_framebuffer);
    if (m_scene_color_texture)
      glDeleteTextures(1, &m_scene_color_texture);
    if (m_scene_color_mask_texture)
      glDeleteTextures(1, &m_scene_color_mask_texture);
    if (m_scene_color_buffer)
      glDeleteRenderbuffers(1, &m_scene_color_buffer);
    if (m_scene_color_mask_buffer)
      glDeleteRenderbuffers(1, &m_scene_color_mask_buffer);
    if (m_scene_depth_buffer)
      glDeleteRenderbuffers(1, &m_scene_depth_buffer);

    m_scene_framebuffer = 0;
    m_scene_resolve_framebuffer = 0;
    m_scene_color_texture = 0;
    m_scene_color_mask_texture = 0;
    m_scene_color_buffer = 0;
    m_scene_color_mask_buffer = 0;
    m_scene_depth_buffer = 0;
    m_scene_framebuffer_ready = false;
  };

  delete_framebuffer_resources();

  GLint maximum_samples = 1;
  glGetIntegerv(GL_MAX_SAMPLES, &maximum_samples);
  const int requested_samples = std::max(
    1, std::min(SCENE_SAMPLE_COUNT, static_cast<int>(maximum_samples)));

  auto allocate_framebuffers = [this, width, height](int samples) {
    glGenFramebuffers(1, &m_scene_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_scene_framebuffer);

    glGenRenderbuffers(1, &m_scene_color_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_scene_color_buffer);
    if (samples > 1)
      glRenderbufferStorageMultisample(
        GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
    else
      glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER,
                              m_scene_color_buffer);

    glGenRenderbuffers(1, &m_scene_color_mask_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_scene_color_mask_buffer);
    if (samples > 1)
      glRenderbufferStorageMultisample(
        GL_RENDERBUFFER, samples, GL_R8, width, height);
    else
      glRenderbufferStorage(GL_RENDERBUFFER, GL_R8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT1,
                              GL_RENDERBUFFER,
                              m_scene_color_mask_buffer);

    glGenRenderbuffers(1, &m_scene_depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_scene_depth_buffer);
    if (samples > 1)
      glRenderbufferStorageMultisample(
        GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, width, height);
    else
      glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER,
                              m_scene_depth_buffer);

    const GLenum scene_draw_buffers[] = { GL_COLOR_ATTACHMENT0,
                                          GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, scene_draw_buffers);
    const bool scene_complete =
      glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    glGenFramebuffers(1, &m_scene_resolve_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_scene_resolve_framebuffer);

    glGenTextures(1, &m_scene_color_texture);
    glBindTexture(GL_TEXTURE_2D, m_scene_color_texture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                            GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D,
                            m_scene_color_texture,
                            0);

    glGenTextures(1, &m_scene_color_mask_texture);
    glBindTexture(GL_TEXTURE_2D, m_scene_color_mask_texture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_R8,
                 width,
                 height,
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                            GL_COLOR_ATTACHMENT1,
                            GL_TEXTURE_2D,
                            m_scene_color_mask_texture,
                            0);

    const GLenum resolve_draw_buffers[] = { GL_COLOR_ATTACHMENT0,
                                            GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, resolve_draw_buffers);
    const bool resolve_complete =
      glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    return scene_complete && resolve_complete;
  };

  int actual_samples = requested_samples;
  bool complete = allocate_framebuffers(actual_samples);
  if (!complete && actual_samples > 1) {
    delete_framebuffer_resources();
    actual_samples = 1;
    complete = allocate_framebuffers(actual_samples);
  }

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
                    static_cast<GLuint>(previous_draw_framebuffer));
  glBindFramebuffer(GL_READ_FRAMEBUFFER,
                    static_cast<GLuint>(previous_read_framebuffer));
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previous_texture));
  glBindRenderbuffer(GL_RENDERBUFFER,
                     static_cast<GLuint>(previous_renderbuffer));

  if (!complete) {
    std::cerr << "Scene framebuffer is incomplete; rendering directly to the window.\n";
    delete_framebuffer_resources();
    return false;
  }

  m_scene_buffer_width = width;
  m_scene_buffer_height = height;
  m_scene_framebuffer_ready = true;

  if (actual_samples > 1)
    std::cout << "Scene antialiasing: " << actual_samples << "x MSAA\n";
  else
    std::cout << "Scene antialiasing: single-sample fallback\n";

  return true;
}

void
App::begin_scene_render()
{
  m_scene_pass_active = false;
  if (!m_window.m_raw_window)
    return;

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(m_window.m_raw_window, &width, &height);
  if (!create_scene_framebuffer(width, height))
    return;

  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_previous_framebuffer);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &m_previous_read_framebuffer);
  glGetIntegerv(GL_VIEWPORT, m_previous_viewport);
  glBindFramebuffer(GL_FRAMEBUFFER, m_scene_framebuffer);
  glViewport(0, 0, m_scene_buffer_width, m_scene_buffer_height);
  const GLfloat clear_mask = 0.0f;
  glClearBufferfv(GL_COLOR, 1, &clear_mask);
  m_scene_pass_active = true;
}

void
App::end_scene_render()
{
  if (!m_scene_pass_active || !m_scene_framebuffer_ready || m_scene_framebuffer == 0 ||
      !m_postprocess_program)
    return;

  GLint previous_program = 0;
  GLint previous_vertex_array = 0;
  GLint previous_active_texture = 0;
  GLint previous_texture = 0;
  GLint previous_mask_texture = 0;
  const GLboolean depth_was_enabled = glIsEnabled(GL_DEPTH_TEST);
  const GLboolean blend_was_enabled = glIsEnabled(GL_BLEND);
  const GLboolean scissor_was_enabled = glIsEnabled(GL_SCISSOR_TEST);
  const GLboolean cull_was_enabled = glIsEnabled(GL_CULL_FACE);
  GLboolean previous_color_mask[4] = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
  glGetBooleanv(GL_COLOR_WRITEMASK, previous_color_mask);
  glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_vertex_array);
  glGetIntegerv(GL_ACTIVE_TEXTURE, &previous_active_texture);
  glActiveTexture(GL_TEXTURE0);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_texture);
  glActiveTexture(GL_TEXTURE1);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_mask_texture);

  glDisable(GL_SCISSOR_TEST);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_scene_framebuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_scene_resolve_framebuffer);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0,
                    0,
                    m_scene_buffer_width,
                    m_scene_buffer_height,
                    0,
                    0,
                    m_scene_buffer_width,
                    m_scene_buffer_height,
                    GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);
  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glDrawBuffer(GL_COLOR_ATTACHMENT1);
  glBlitFramebuffer(0,
                    0,
                    m_scene_buffer_width,
                    m_scene_buffer_height,
                    0,
                    0,
                    m_scene_buffer_width,
                    m_scene_buffer_height,
                    GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, m_scene_buffer_width, m_scene_buffer_height);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_CULL_FACE);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  m_postprocess_program->bind();
  glBindVertexArray(m_postprocess_vertex_array);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_scene_color_texture);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_scene_color_mask_texture);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previous_mask_texture));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previous_texture));
  glActiveTexture(static_cast<GLenum>(previous_active_texture));
  glBindVertexArray(static_cast<GLuint>(previous_vertex_array));
  glUseProgram(static_cast<GLuint>(previous_program));
  if (depth_was_enabled)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  if (blend_was_enabled)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  if (scissor_was_enabled)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
  if (cull_was_enabled)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
  glColorMask(previous_color_mask[0],
              previous_color_mask[1],
              previous_color_mask[2],
              previous_color_mask[3]);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
                     static_cast<GLuint>(m_previous_framebuffer));
  glBindFramebuffer(GL_READ_FRAMEBUFFER,
                     static_cast<GLuint>(m_previous_read_framebuffer));
  glViewport(m_previous_viewport[0],
             m_previous_viewport[1],
             m_previous_viewport[2],
             m_previous_viewport[3]);
  m_scene_pass_active = false;
}

void
App::destroy_scene_render_resources()
{
  if (m_scene_framebuffer)
    glDeleteFramebuffers(1, &m_scene_framebuffer);
  if (m_scene_resolve_framebuffer)
    glDeleteFramebuffers(1, &m_scene_resolve_framebuffer);
  if (m_scene_color_texture)
    glDeleteTextures(1, &m_scene_color_texture);
  if (m_scene_color_mask_texture)
    glDeleteTextures(1, &m_scene_color_mask_texture);
  if (m_scene_color_buffer)
    glDeleteRenderbuffers(1, &m_scene_color_buffer);
  if (m_scene_color_mask_buffer)
    glDeleteRenderbuffers(1, &m_scene_color_mask_buffer);
  if (m_scene_depth_buffer)
    glDeleteRenderbuffers(1, &m_scene_depth_buffer);
  if (m_postprocess_vertex_array)
    glDeleteVertexArrays(1, &m_postprocess_vertex_array);
  m_postprocess_program.reset();
  m_scene_framebuffer = 0;
  m_scene_resolve_framebuffer = 0;
  m_scene_color_texture = 0;
  m_scene_color_mask_texture = 0;
  m_scene_color_buffer = 0;
  m_scene_color_mask_buffer = 0;
  m_scene_depth_buffer = 0;
  m_postprocess_vertex_array = 0;
  m_scene_framebuffer_ready = false;
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
