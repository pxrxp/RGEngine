#pragma once

#include "Camera.h"
#include "Renderer.h"
#include "repos/MeshRepo.h"
#include "repos/MaterialRepo.h"
#include "Window.h"

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
};

inline bool App::m_first_mouse_update = true;
