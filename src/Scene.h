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
  SceneObjectStrongPtr m_player;
  SceneObjectStrongPtr m_skybox;
  SceneObjectStrongPtr m_enemy_root;

  // Character Controller State
  float m_player_vy = 0.0f;
  bool m_player_grounded = true;
  static constexpr float PLAYER_EYE_HEIGHT = 1.7f;
  static constexpr float PLAYER_RADIUS = 0.35f;
  static constexpr float MAX_STEP_HEIGHT = 0.55f;

  bool m_space_was_pressed = false;
  float m_jump_cooldown = 0.0f;

  bool m_mouse_was_pressed = false;
  virtual void handle_shooting();
  void update_character_controller(float dt);

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

  App& get_app_cache() { return m_app_cache; }

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

  void set_enemy_root(SceneObjectStrongPtr root) { m_enemy_root = root; }

  SceneObjectStrongPtr get_scene_ptr() const;
  SceneObjectStrongPtr get_player() const { return m_player; }
};
