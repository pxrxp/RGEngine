#include "App.h"
#include "Renderer.h"
#include "definitions.h"
#include "scenes.h"

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
    std::cout << app.get_camera().get_bounding_box().min << std::endl << app.get_camera().get_bounding_box().max << std::endl << std::endl << std::endl;
    test.update_physics(app);
    app.get_renderer().submit(test.get_scene_ptr());
    app.get_renderer().render();

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
}
