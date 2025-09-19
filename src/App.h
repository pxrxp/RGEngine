#pragma once
#include "Window.h"

class App {
private:
  Window window;
  bool imgui_initialized = false;

  float delta_time;
  double last_frame_time, current_frame_time;

public:
  ~App();
  bool is_running() const;
  void init_window();
  Window &get_window();
  void update_window();
  void init_debug_gui();
  void render_debug_gui();
  float get_delta_time() const;
};
