#pragma once
#include "App.h"
#include "Renderer.h"
#include "SceneObject.h"

class Scene
{
private:
  void collect_physics_objects_recursive(SceneObjectPtr object,
                                         std::vector<SceneObjectPtr>& objects);
protected:
  App& m_app_cache;
  SceneObjectPtr m_scene_ptr;

  void set_view_matrix(RenderPacketPtr render_packet,
                       const char* view_uniform_name = "view",
                       const glm::mat4& view_matrix = glm::mat4(1.f));

  void set_projection_matrix(RenderPacketPtr render_packet,
                             const char* proj_uniform_name = "proj",
                             const glm::mat4& proj_matrix = glm::mat4(1.f));

  void set_model_matrix(RenderPacketPtr render_packet,
                        const glm::mat4& model_matrix,
                        const char* uniform_name = "model");

  void display_AABB(SceneObjectPtr object,
                    bool show_controller = false);

public:
  Scene(App& app)
    : m_app_cache(app)
    , m_scene_ptr(std::make_shared<SceneObject>())
  {
  }

  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  Scene(Scene&&) = default;
  Scene& operator=(Scene&&) = delete;

  virtual ~Scene() = default;
  virtual void init();

  std::vector<SceneObjectPtr> get_all_physics_objects();
  void step_simulation(float fixed_step);
  void update_physics();

  void submit_to_renderer();

  SceneObjectPtr get_scene_ptr() const;
};
