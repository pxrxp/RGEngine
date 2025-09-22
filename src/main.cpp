#include "App.h"
#include "Renderer.h"
#include "definitions.h"
#include "util.h"

#include "scenes.h"
#include <glm/glm.hpp>

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

#ifndef NDEBUG
  MatrixEditors all_matrix_editors;
  // all_matrix_editors.add("Model", model);
#endif

  while (app.is_running()) {
    app.update();
    app.init_debug_gui();

#ifndef NDEBUG
    float sensitivity = app.get_camera().get_sensitivity();
    ImGui::Begin("Debug");
    ImGui::Text("DeltaTime: %f", app.get_delta_time());
    ImGui::End();
#endif

    app.get_window().clear(COLOR_BLACK,
                           GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    app.get_renderer().submit(test.get_scene_ptr());
    app.get_renderer().render();

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
}
