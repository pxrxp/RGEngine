#include "App.h"
#include "Octree.h"
#include "TestScene.h"
#include "components/BoundingBox.h"
#include "util/definitions.h"
#include <memory>

int
main()
{
  Octree<int> octree(BoundingBox{ { 0, 0, 0 }, { 100, 100, 100 } });
  octree.insert(1, { 10, 10, 10 });
  octree.insert(2, { 15, 15, 15 });
  octree.insert(3, { 20, 20, 20 });
  octree.insert(4, { 25, 25, 25 });
  octree.insert(5, { 5, 5, 5 });
  octree.insert(6, { 30, 30, 30 });
  octree.insert(7, { 35, 35, 35 });
  octree.insert(8, { 40, 40, 40 });

  std::cout << "--- After 8 insertions (leaf should be full) ---\n";
  std::cout << octree << std::endl;
  octree.insert(9, { 80, 80, 80 });

  std::cout << "--- After 9th insertion (should be subdivided) ---\n";
  std::cout << octree << std::endl;

  octree.insert(10, { 90, 10, 10 }); // +X, -Y, -Z octant
  octree.insert(11, { 10, 90, 10 }); // -X, +Y, -Z octant
  octree.insert(12, { 10, 10, 90 }); // -X, -Y, +Z octant

  std::cout << "--- After more insertions ---\n";
  std::cout << octree << std::endl;

  auto app = std::make_unique<App>();
  app->init();

  {
    auto test_scene = std::make_shared<TestScene>(*app);
    test_scene->init();

    while (app->is_running()) {
      app->update();
      app->init_debug_gui();

      app->get_window().clear(COLOR_BLACK,
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      test_scene->update_physics();
      test_scene->submit_to_renderer();
      app->get_renderer().render();

      app->render_debug_gui();
      app->get_window().swap_buffers();
    }
  }

  app->cleanup();
  app.reset();
  return EXIT_SUCCESS;
}
