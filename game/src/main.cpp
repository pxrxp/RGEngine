#include <engine/App.h>
#include <engine/util/definitions.h>
#include <rivalgrounds/RivalGrounds.h>
#include <memory>

int
main()
{
  App app(AppConfig{ .window_title = "RivalGrounds" });
  app.init();

  app.get_window().set_cursor_mode(GLFW_CURSOR_DISABLED);
  app.set_post_process(ShaderProgramDescriptor{
    .program_name = "grayscale_postprocess",
    .shaders = { "postprocess.vert", "grayscale.frag" } });

  {
    auto game_scene = std::make_shared<RivalGrounds>(app);
    game_scene->init();

    while (app.is_running()) {
      app.update();
      app.init_debug_gui();
      app.begin_scene_render();
      app.get_window().clear(RGBA_BLACK,
                             GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      game_scene->update_physics();
      game_scene->submit_to_renderer();
      app.get_renderer().render();
      app.end_scene_render();

      app.render_debug_gui();
      app.get_window().swap_buffers();
    }
  }

  app.cleanup();
  return EXIT_SUCCESS;
}
