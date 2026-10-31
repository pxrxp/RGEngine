#pragma once

#include "Camera.h"
#include "Renderer.h"
#include "repos/MeshRepo.h"
#include "repos/MaterialRepo.h"
#include "Window.h"
#include "util/opengl.h"

struct GLFWwindow;
class Window;
class Camera;

class App
{
private:
  Window m_window;
  Camera m_camera;
  Renderer m_renderer;

  static bool m_first_mouse_update;
  bool m_imgui_initialized = false;

  GLuint m_scene_framebuffer = 0;
  GLuint m_scene_resolve_framebuffer = 0;
  GLuint m_scene_color_texture = 0;
  GLuint m_scene_color_mask_texture = 0;
  GLuint m_scene_color_buffer = 0;
  GLuint m_scene_color_mask_buffer = 0;
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
  bool create_postprocess_program();
  void destroy_scene_render_resources();

  float m_delta_time;
  double m_last_frame_time, m_current_frame_time;

  void update_delta_time();

  void register_callbacks();
  static void framebuffer_size_callback(GLFWwindow* window,
                                        int width,
                                        int height);
  static void key_callback(GLFWwindow* window,
                           int key,
                           int scancode,
                           int action,
                           int mods);
  static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);

public:
  MeshRepo mesh_repo;
  MaterialRepo material_repo;

  App() = default;
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

  void init_debug_gui();
  void render_debug_gui();
  void begin_scene_render();
  void end_scene_render();
};

inline bool App::m_first_mouse_update = true;
