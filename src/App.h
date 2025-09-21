#pragma once
#include "Camera.h"
#include "Window.h"
#include "Renderer.h"
#include "Repo.h"
#include "opengl.h"

class App {
private:
  Window window;
  Camera camera;
  Renderer renderer;

  bool imgui_initialized = false;

  float delta_time;
  double last_frame_time, current_frame_time;

  void update_window_size();
  void update_mouse_position();
  void update_delta_time();
  void update_camera();

  bool is_key_pressed(int key) const;
  glm::vec3 get_camera_move_dir() const;

  void register_callbacks();
  static void framebuffer_size_callback(GLFWwindow *window, int width,
                                        int height);
  static void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods);

public:
  ShaderProgramRepo shader_program_repo;
  MaterialRepo material_repo;
  MeshRepo mesh_repo;

  ~App();
  void init();
  void update();

  bool is_running() const;

  Window &get_window();
  Camera &get_camera();
  Renderer &get_renderer();

  float get_delta_time() const;

  void init_debug_gui();
  void render_debug_gui();

};
