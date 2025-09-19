#pragma once
#include "Window.h"
#include "Camera.h"

class App {
private:
  Window window;
  Camera camera;
  bool imgui_initialized = false;

  float delta_time;
  double last_frame_time, current_frame_time;

  void update_window_size();
  void update_mouse_position();
  void update_delta_time();
  void update_camera();

public:
  ~App();
  void init();
  void update();

  bool is_running() const;

  Window &get_window();
  Camera &get_camera();

  void init_debug_gui();
  void render_debug_gui();

  float get_delta_time() const;
};
