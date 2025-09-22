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

  app.get_camera().setup({2., 3., 2.}, {0., 0., 0.},
                         app.get_window().get_aspect_ratio(), 20.0f,
                         {0.05f, 0.05f, 0.05f});

  TestScene test(app);

  while (app.is_running()) {
    app.update();
    app.init_debug_gui();

    app.get_window().clear(COLOR_BLACK,
                           GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    test.update_physics();
    app.get_renderer().submit(test.get_scene_ptr());
    app.get_renderer().render();

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
}
