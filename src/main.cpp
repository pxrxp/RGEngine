#include "App.h"
#include "util/definitions.h"
#include "scenes.h"
#include <cstdlib>

int
main()
{
  App app;
  app.init();

  auto test_scene = std::make_shared<TestScene>(app);
  test_scene->init();

  while (app.is_running()) {
    app.update();
    app.init_debug_gui();

    app.get_window().clear(COLOR_BLACK,
                           GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    test_scene->update_physics();
    test_scene->submit_to_renderer();
    app.get_renderer().render();

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
  return EXIT_SUCCESS;
}
