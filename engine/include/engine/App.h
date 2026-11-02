#pragma once

#include <engine/Camera.h>
#include <engine/Renderer.h>
#include <engine/repos/MaterialRepo.h>
#include <engine/repos/MeshRepo.h>
#include <engine/repos/ShaderProgramRepo.h>
#include <engine/Window.h>
#include <engine/util/opengl.h>
#include <string>

struct GLFWwindow;
class Window;
class Camera;

struct AppConfig
{
  std::string window_title = "Application";
  // Zero picks half of the primary monitor's resolution.
  int width = 0;
  int height = 0;
  bool decorated = true;
  int msaa_samples = 4;
};

class App
{
private:
  AppConfig m_config;

  Window m_window;
  Camera m_camera;
  Renderer m_renderer;

  bool m_imgui_initialized = false;

  // Offscreen scene target. Colour attachment 0 feeds the post-process pass;
  // attachment 1 is a free auxiliary channel the game may fill in its shaders.
  GLuint m_scene_framebuffer = 0;
  GLuint m_scene_resolve_framebuffer = 0;
  GLuint m_scene_color_texture = 0;
  GLuint m_scene_aux_texture = 0;
  GLuint m_scene_color_buffer = 0;
  GLuint m_scene_aux_buffer = 0;
  GLuint m_scene_depth_buffer = 0;
  ShaderProgramStrongPtr m_postprocess_program;
  GLuint m_postprocess_vertex_array = 0;
  int m_scene_buffer_width = 0;
  int m_scene_buffer_height = 0;
  bool m_scene_framebuffer_ready = false;
  bool m_scene_pass_active = false;
  GLint m_previous_framebuffer = 0;
  GLint m_previous_read_framebuffer = 0;
  GLint m_previous_viewport[4] = { 0, 0, 0, 0 };

  bool create_scene_framebuffer(int width, int height);
  void destroy_scene_render_resources();

  float m_delta_time = 0.f;
  double m_last_frame_time = 0.0;
  double m_current_frame_time = 0.0;

  void update_delta_time();

  void register_callbacks();
  static void framebuffer_size_callback(GLFWwindow* window,
                                        int width,
                                        int height);
  static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);

public:
  MeshRepo mesh_repo;
  MaterialRepo material_repo;

  explicit App(AppConfig config = {});
  App(const App&) = delete;
  App& operator=(const App&) = delete;
  ~App();

  void init();
  void update();
  void cleanup();

  bool is_running() const;

  Window& get_window();
  Camera& get_camera();
  Renderer& get_renderer();

  float get_delta_time() const;

  // Fullscreen post-process pass drawn after the scene is resolved. The
  // program must sample the resolved scene through the uniform names
  // "u_scene" (colour) and "u_scene_aux" (auxiliary attachment). Without a
  // program the resolved scene is copied straight to the window.
  bool set_post_process(const ShaderProgramDescriptor& program);

  void init_debug_gui();
  void render_debug_gui();
  void begin_scene_render();
  void end_scene_render();
};
