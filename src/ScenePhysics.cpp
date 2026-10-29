#include "Scene.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace {
bool
is_static_object(const SceneObject& object)
{
  const auto& physics = object.get_physics_component();
  return std::isinf(physics.get_mass()) || !physics.is_enabled();
}

struct Cell
{
  int x;
  int z;
  bool operator==(const Cell& other) const { return x == other.x && z == other.z; }
};

struct CellHash
{
  std::size_t operator()(const Cell& cell) const
  {
    const std::uint64_t x = static_cast<std::uint32_t>(cell.x);
    const std::uint64_t z = static_cast<std::uint32_t>(cell.z);
    std::uint64_t h = (x << 32) | z;
    h ^= h >> 30;
    h *= 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 27;
    h *= 0x94d049bb133111ebULL;
    h ^= h >> 31;
    return static_cast<std::size_t>(h);
  }
};

constexpr float GRID_CELL_SIZE = 2.2f;
constexpr float MAX_STACK_CORRECTION = 1.0f;

Cell
cell_for(const glm::vec3& p)
{
  return { static_cast<int>(std::floor(p.x / GRID_CELL_SIZE)),
           static_cast<int>(std::floor(p.z / GRID_CELL_SIZE)) };
}

void
stop_downward_motion(PhysicsComponent& physics, float horizontal_damping)
{
  glm::vec3 momentum = physics.get_linear_momentum();
  if (momentum.y < 0.0f)
    momentum.y = 0.0f;
  momentum.x *= horizontal_damping;
  momentum.z *= horizontal_damping;
  physics.set_linear_momentum(momentum);
  physics.set_angular_momentum(glm::vec3(0.0f));
}
} // namespace

void
Scene::collect_physics_objects_recursive(
  SceneObjectStrongPtr object,
  std::vector<SceneObjectStrongPtr>& objects)
{
  if (object->get_physics_component().is_enabled())
    objects.push_back(object);

  for (auto& child : *object)
    collect_physics_objects_recursive(child, objects);
}

void
Scene::collect_colliders_recursive(SceneObjectStrongPtr object,
                                   std::vector<BoundingBox>& colliders)
{
  if (!object->is_collidable())
    return;

  if (object->get_render_packet().has_value())
    colliders.push_back(object->get_world_AABB());

  for (auto& child : *object)
    collect_colliders_recursive(child, colliders);
}

std::vector<SceneObjectStrongPtr>
Scene::get_all_physics_objects()
{
  std::vector<SceneObjectStrongPtr> objects;
  objects.reserve(256);
  for (auto& child : *m_scene_ptr)
    collect_physics_objects_recursive(child, objects);
  return objects;
}

void
Scene::step_simulation()
{
  auto objects = get_all_physics_objects();

          // Never auto-step while in jump cooldown or moving upward!
          float step_diff = b_max.y - feet_y;
          if (m_player_grounded && m_jump_cooldown <= 0.0f &&
              step_diff > 0.05f && step_diff <= MAX_STEP_HEIGHT) {
            if (b_max.y > out_step_y)
              out_step_y = b_max.y;
          } else {
            return false;
          }
        }
      }
      return true;
    };

    float step_x = feet_y;
    if (test_axis(player_pos.x + delta.x, player_pos.z, step_x)) {
      player_pos.x += delta.x;
      if (step_x > feet_y)
        feet_y = step_x;
    }

    float step_z = feet_y;
    if (test_axis(player_pos.x, player_pos.z + delta.z, step_z)) {
      player_pos.z += delta.z;
      if (step_z > feet_y)
        feet_y = step_z;
    }

    // Only apply step elevation if grounded and not jumping
    if (m_player_grounded && m_jump_cooldown <= 0.0f &&
        feet_y > player_pos.y - PLAYER_EYE_HEIGHT) {
      player_pos.y = feet_y + PLAYER_EYE_HEIGHT;
    }
  }

  // Jump: Only triggers once on initial key-press (no continuous boost)
  bool space_is_pressed = window.is_key_pressed(GLFW_KEY_SPACE);
  if (space_is_pressed && !m_space_was_pressed && m_player_grounded &&
      m_jump_cooldown <= 0.0f) {
    m_player_vy = 6.4f;
    m_player_grounded = false;
    m_jump_cooldown = 0.25f;
  }
  m_space_was_pressed = space_is_pressed;

  // Gravity & Landing (only land while falling down)
  m_player_vy -= 18.0f * dt;
  player_pos.y += m_player_vy * dt;
  feet_y = player_pos.y - PLAYER_EYE_HEIGHT;

  if (m_player_vy <= 0.0f && m_jump_cooldown <= 0.0f) {
    float ground_y = 0.0f;
    for (const auto& aabb : colliders) {
      const glm::vec3& b_min = aabb.get_min();
      const glm::vec3& b_max = aabb.get_max();

      bool inside_x =
        (player_pos.x >= b_min.x - 0.15f) && (player_pos.x <= b_max.x + 0.15f);
      bool inside_z =
        (player_pos.z >= b_min.z - 0.15f) && (player_pos.z <= b_max.z + 0.15f);

      if (inside_x && inside_z) {
        if (b_max.y <= feet_y + 0.35f && b_max.y > ground_y) {
          ground_y = b_max.y;
        }
      }
    }

    if (feet_y <= ground_y) {
      player_pos.y = ground_y + PLAYER_EYE_HEIGHT;
      m_player_vy = 0.0f;
      m_player_grounded = true;
    } else {
      m_player_grounded = false;
    }
  }

  m_player->get_local_transform().set_position(player_pos);
  m_app_cache.get_camera().set_view_matrix_dirty();
}

void
Scene::step_simulation()
{
  // Character controller (player walking, stepping, jumping)
  update_character_controller(FIXED_STEP);

  // Rigidbodies
  std::vector<SceneObjectStrongPtr> objects = get_all_physics_objects();

  for (auto& object : objects) {
    auto& physics = object->get_physics_component();
    physics.clear_forces();

    if (physics.has_gravity() && !is_static_object(*object)) {
      const glm::vec3 gravity_force =
        glm::vec3(0.0f, -9.81f, 0.0f) * physics.get_mass();
      physics.apply_force(gravity_force);
    }

    const glm::vec3 velocity = physics.get_velocity();
    if (std::isfinite(velocity.x) && std::isfinite(velocity.y) &&
        std::isfinite(velocity.z)) {
      physics.apply_force(-physics.get_drag() * velocity);
    }

    physics.integrate_velocity();
  }

  for (auto& object : objects) {
    if (!is_static_object(*object)) {
      object->get_physics_component().integrate_position();
    }
  }

  for (auto& object : objects) {
    resolve_floor_plane(*object);
  }

  for (size_t i = 0; i < objects.size(); ++i) {
    for (size_t j = i + 1; j < objects.size(); ++j) {
      SceneObjectStrongPtr& a = objects[i];
      SceneObjectStrongPtr& b = objects[j];

      if (is_static_object(*a) && is_static_object(*b))
        continue;

      const BoundingBox& aabb_a = a->get_world_AABB();
      const BoundingBox& aabb_b = b->get_world_AABB();

      if (aabb_a && aabb_b && aabb_a.intersects(aabb_b)) {
        resolve_penetration(*a, *b);
      }
    }
  }
}

void
Scene::update_physics()
{
  static double accumulator = 0.0;
  accumulator += m_app_cache.get_delta_time();

  if (accumulator > 0.2)
    accumulator = 0.2;

  while (accumulator >= Scene::FIXED_STEP) {
    step_simulation();
    accumulator -= Scene::FIXED_STEP;
  }
}
