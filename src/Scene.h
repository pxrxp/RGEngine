#pragma once
#include "App.h"
#include "Renderer.h"
#include "SceneObject.h"
#include <cassert>
#include <optional>

template<typename F>
auto
capture_weak(SceneObjectStrongPtr object_strong, F&& f)
{
  SceneObjectWeakPtr obj_weak = object_strong;
  return [obj_weak, f = std::forward<F>(f)]() mutable {
    SceneObjectStrongPtr self = obj_weak.lock();

    if (self) {
      auto packet_strong_opt = self->get_render_packet();
      if (packet_strong_opt)
        f(self, packet_strong_opt.value());
    }
  };
}

class Scene
{
private:
  void collect_physics_objects_recursive(
    SceneObjectStrongPtr object,
    std::vector<SceneObjectStrongPtr>& objects);

protected:
  App& m_app_cache;
  SceneObjectStrongPtr m_scene_ptr;

  void set_view_matrix(RenderPacketStrongPtr render_packet,
                       const char* view_uniform_name = "u_view",
                       const glm::mat4& view_matrix = glm::mat4(1.f));

  void set_projection_matrix(RenderPacketStrongPtr render_packet,
                             const char* proj_uniform_name = "u_proj",
                             const glm::mat4& proj_matrix = glm::mat4(1.f));

  void set_model_matrix(RenderPacketStrongPtr render_packet,
                        const glm::mat4& model_matrix,
                        const char* uniform_name = "u_model");

  void debug_object(std::weak_ptr<SceneObject> weak_object, const char* header);
  void debug_camera();
  void setup_skybox(const SceneObjectStrongPtr& skybox,
                    const std::string& texture_name);

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

  std::vector<SceneObjectStrongPtr> get_all_physics_objects();
  void step_simulation(float fixed_step);
  void update_physics();

  void submit_to_renderer();

  SceneObjectStrongPtr get_scene_ptr() const;
};
