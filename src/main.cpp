#include "App.h"
#include "Renderer.h"
#include "definitions.h"
#include "scenes.h"

#define GL_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#include <iostream>

#ifndef NDEBUG
#include <imgui.h>
#endif

int main() {
  App app;
  app.init();

  TestScene test(app);

  while (app.is_running()) {
    app.update();
    app.init_debug_gui();

    app.get_window().clear(COLOR_BLACK,
                           GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    test.update_physics(app);
    app.get_renderer().submit(test.get_scene_ptr());
    app.get_renderer().render();
    std::cout << app.get_camera().get_bounding_box().min << ", " << app.get_camera().get_bounding_box().max << std::endl;

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
}
