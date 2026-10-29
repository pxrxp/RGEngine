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

  for (auto& object : objects) {
    auto& physics = object->get_physics_component();
    physics.clear_forces();

    if (physics.has_gravity() && !is_static_object(*object))
      physics.apply_force(glm::vec3(0.0f, -14.0f, 0.0f) * physics.get_mass());

    physics.set_linear_momentum(physics.get_linear_momentum() * 0.95f);
    physics.set_angular_momentum(physics.get_angular_momentum() * 0.85f);
    physics.integrate_velocity();
  }

  for (auto& object : objects) {
    if (!is_static_object(*object))
      object->get_physics_component().integrate_position();
  }

  // Broad phase: bucket dynamic gravity objects in XZ cells. A crate only
  // checks its own cell and the eight adjacent cells, rather than all 256.
  using BucketMap = std::unordered_map<Cell, std::vector<std::size_t>, CellHash>;
  BucketMap buckets;
  buckets.reserve(objects.size() * 2);

  for (std::size_t i = 0; i < objects.size(); ++i) {
    if (is_static_object(*objects[i]) ||
        !objects[i]->get_physics_component().has_gravity())
      continue;
    buckets[cell_for(objects[i]->get_local_transform().get_position())].push_back(i);
  }

  // Narrow phase: visit each candidate pair once, regardless of object order.
  for (std::size_t i = 0; i < objects.size(); ++i) {
    if (is_static_object(*objects[i]) ||
        !objects[i]->get_physics_component().has_gravity())
      continue;

    const Cell cell = cell_for(objects[i]->get_local_transform().get_position());

    for (int dz = -1; dz <= 1; ++dz) {
      for (int dx = -1; dx <= 1; ++dx) {
        const auto it = buckets.find({ cell.x + dx, cell.z + dz });
        if (it == buckets.end())
          continue;

        for (const std::size_t j : it->second) {
          if (j <= i)
            continue;

          const BoundingBox a = objects[i]->get_world_AABB();
          const BoundingBox b = objects[j]->get_world_AABB();

          const float overlapX =
            std::min(a.get_max().x, b.get_max().x) -
            std::max(a.get_min().x, b.get_min().x);
          const float overlapZ =
            std::min(a.get_max().z, b.get_max().z) -
            std::max(a.get_min().z, b.get_min().z);
          if (overlapX <= 0.0f || overlapZ <= 0.0f)
            continue;

          // If one crate has penetrated the top face of the other, lift the
          // upper crate. This is symmetric, so spawn/index order does not matter.
          std::size_t upperIndex = objects.size();
          float correction = 0.0f;

          const float aBottomIntoBTop = b.get_max().y - a.get_min().y;
          if (aBottomIntoBTop > 0.0f &&
              aBottomIntoBTop < MAX_STACK_CORRECTION &&
              a.get_max().y > b.get_max().y) {
            upperIndex = i;
            correction = aBottomIntoBTop;
          } else {
            const float bBottomIntoATop = a.get_max().y - b.get_min().y;
            if (bBottomIntoATop > 0.0f &&
                bBottomIntoATop < MAX_STACK_CORRECTION &&
                b.get_max().y > a.get_max().y) {
              upperIndex = j;
              correction = bBottomIntoATop;
            }
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
