#pragma once
#include "App.h"
#include "Renderer.h"
#include "SceneObject.h"
#include <cassert>
#include <optional>
#include <vector>

template<typename F>
auto
capture_weak(SceneObjectStrongPtr object_strong, F&& f)
{
  SceneObjectWeakPtr obj_weak = object_strong;
  return [obj_weak, f = std::forward<F>(f)]() mutable {
    SceneObjectStrongPtr self = obj_weak.lock();

    if (self && self->visible) {
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

  void debug_object(std::weak_ptr<SceneObject> weak_object, const char* header);
  void debug_camera();
  void setup_skybox(const SceneObjectStrongPtr& skybox,
                    const std::string& texture_name);

  virtual bool blocks_vertical_capsule(float x,
                                       float z,
                                       float from_x,
                                       float from_z,
                                       float feet_y,
                                       float head_y,
                                       float radius) const
  {
    (void)x;
    (void)z;
    (void)from_x;
    (void)from_z;
    (void)feet_y;
    (void)head_y;
    (void)radius;
    return false;
  }

  virtual bool static_mesh_ray_hit(const glm::vec3& origin,
                                   const glm::vec3& direction,
                                   float max_distance,
                                   float& out_distance) const
  {
    (void)origin;
    (void)direction;
    (void)max_distance;
    (void)out_distance;
    return false;
  }

  virtual bool find_ground_surface_for_actor(float x,
                                             float z,
                                             float max_y,
                                             float min_y,
                                             float& out_y) const
  {
    return find_ground_surface(x, z, max_y, min_y, out_y);
  }

  virtual bool find_ground_surface(float x,
                                   float z,
                                   float max_y,
                                   float min_y,
                                   float& out_y) const
  {
    (void)x;
    (void)z;
    (void)max_y;
    (void)min_y;
    (void)out_y;
    return false;
  }

  virtual bool find_landing_surface(float x,
                                    float z,
                                    float radius,
                                    float previous_bottom,
                                    float current_bottom,
                                    float& out_y) const
  {
    (void)radius;
    return find_ground_surface(x,
                               z,
                               previous_bottom + 0.05f,
                               current_bottom - 0.05f,
                               out_y);
  }

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

  App& get_app_cache() { return m_app_cache; }

  bool blocks_vertical_capsule_for_actor(float x,
                                         float z,
                                         float from_x,
                                         float from_z,
                                         float feet_y,
                                         float head_y,
                                         float radius) const
  {
    return blocks_vertical_capsule(
      x, z, from_x, from_z, feet_y, head_y, radius);
  }

  bool raycast_static_geometry(const glm::vec3& origin,
                               const glm::vec3& direction,
                               float max_distance,
                               float& out_distance) const
  {
    return static_mesh_ray_hit(origin, direction, max_distance, out_distance);
  }

  bool find_actor_ground_surface(float x,
                                 float z,
                                 float max_y,
                                 float min_y,
                                 float& out_y) const
  {
    return find_ground_surface_for_actor(x, z, max_y, min_y, out_y);
  }

  void set_view_matrix(RenderPacketStrongPtr render_packet,
                       const char* view_uniform_name = "u_view",
                       const glm::mat4& view_matrix = glm::mat4(1.f));

  void set_projection_matrix(RenderPacketStrongPtr render_packet,
                             const char* proj_uniform_name = "u_proj",
                             const glm::mat4& proj_matrix = glm::mat4(1.f));

  void set_model_matrix(RenderPacketStrongPtr render_packet,
                        const glm::mat4& model_matrix,
                        const char* uniform_name = "u_model");

  void collect_colliders_recursive(SceneObjectStrongPtr object,
                                   std::vector<BoundingBox>& colliders);

  std::vector<SceneObjectStrongPtr> get_all_physics_objects();

  static constexpr float FIXED_STEP = 1.f / 60.f;
  void step_simulation();
  virtual void update_physics();

  void submit_to_renderer();

  SceneObjectStrongPtr get_scene_ptr() const;
};
