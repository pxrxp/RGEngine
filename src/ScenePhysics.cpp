#include "Scene.h"

namespace {
bool
is_static_object(const SceneObject& object)
{
  const auto& physics = object.get_physics_component();
  return std::isinf(physics.get_mass()) || !physics.is_enabled();
}

void
resolve_floor_plane(SceneObject& object)
{
  if (is_static_object(object))
    return;

  auto& physics = object.get_physics_component();
  if (!physics.has_gravity())
    return;

  const auto& aabb = object.get_world_AABB();

  const float half_height = 0.5f * (aabb.get_max().y - aabb.get_min().y);
  const float bottom =
    object.get_local_transform().get_position().y - half_height;

  if (bottom < 0.0f) {
    const float correction = -bottom;
    object.get_local_transform().translate(glm::vec3(0.0f, correction, 0.0f));

    glm::vec3 momentum = physics.get_linear_momentum();
    momentum.y = std::max(0.0f, momentum.y);
    physics.set_linear_momentum(momentum);
  }
}

void
resolve_penetration(SceneObject& a, SceneObject& b)
{
  const BoundingBox& aabb_a = a.get_world_AABB();
  const BoundingBox& aabb_b = b.get_world_AABB();

  glm::vec3 overlap(std::min(aabb_a.get_max().x, aabb_b.get_max().x) -
                      std::max(aabb_a.get_min().x, aabb_b.get_min().x),
                    std::min(aabb_a.get_max().y, aabb_b.get_max().y) -
                      std::max(aabb_a.get_min().y, aabb_b.get_min().y),
                    std::min(aabb_a.get_max().z, aabb_b.get_max().z) -
                      std::max(aabb_a.get_min().z, aabb_b.get_min().z));

  if (overlap.x <= 0.0f && overlap.y <= 0.0f && overlap.z <= 0.0f)
    return;

  glm::vec3 center_a = 0.5f * (aabb_a.get_min() + aabb_a.get_max());
  glm::vec3 center_b = 0.5f * (aabb_b.get_min() + aabb_b.get_max());
  glm::vec3 delta = center_b - center_a;

  glm::vec3 mtv(0.0f);

  if (overlap.x > 0.0f && overlap.x <= overlap.y && overlap.x <= overlap.z) {
    mtv.x = overlap.x;
    if (delta.x < 0.0f)
      mtv.x = -mtv.x;
  } else if (overlap.y > 0.0f && overlap.y <= overlap.z) {
    mtv.y = overlap.y;
    if (delta.y < 0.0f)
      mtv.y = -mtv.y;
  } else if (overlap.z > 0.0f) {
    mtv.z = overlap.z;
    if (delta.z < 0.0f)
      mtv.z = -mtv.z;
  }

  if (glm::length(mtv) <= 0.0f)
    return;

  float mass_a = a.get_physics_component().get_mass();
  float mass_b = b.get_physics_component().get_mass();

  float move_a = 0.0f;
  float move_b = 0.0f;
  float total = 0.0f;

  if (std::isfinite(mass_a) && mass_a > 0.0001f) {
    move_a = mass_a;
    total += mass_a;
  }

  if (std::isfinite(mass_b) && mass_b > 0.0001f) {
    move_b = mass_b;
    total += mass_b;
  }

  if (total > 0.0001f) {
    move_a /= total;
    move_b /= total;
  }

  a.get_local_transform().translate(-mtv * move_a);
  b.get_local_transform().translate(mtv * move_b);
}

void
resolve_velocity(SceneObject& a, SceneObject& b)
{
  // TODO
}
} // namespace

void
Scene::collect_physics_objects_recursive(
  SceneObjectStrongPtr object,
  std::vector<SceneObjectStrongPtr>& objects)
{
  if (object->get_physics_component().is_enabled())
    objects.push_back(object);

  for (auto& child : *object) {
    collect_physics_objects_recursive(child, objects);
  }
}

std::vector<SceneObjectStrongPtr>
Scene::get_all_physics_objects()
{
  std::vector<SceneObjectStrongPtr> objects;
  for (auto& child : *m_scene_ptr) {
    collect_physics_objects_recursive(child, objects);
  }
  return objects;
}

void
Scene::step_simulation()
{
  std::vector<SceneObjectStrongPtr> objects = get_all_physics_objects();

  for (auto& object : objects) {
    auto& physics = object->get_physics_component();

    physics.clear_forces();

    if (physics.has_gravity()) {
      const glm::vec3 gravity_force =
        glm::vec3(0.0f, -9.8f, 0.0f) * physics.get_mass();
      physics.apply_force(gravity_force);
    }

    const glm::vec3 velocity = physics.get_velocity();
    if (std::isfinite(velocity.x) && std::isfinite(velocity.y) &&
        std::isfinite(velocity.z))
      physics.apply_force(-physics.get_drag() * physics.get_velocity());

    physics.integrate_velocity();
  }

  for (auto& object : objects) {
    object->get_physics_component().integrate_position();
  }

  for (auto& object : objects) {
    resolve_floor_plane(*object);
  }

  for (size_t i = 0; i < objects.size(); ++i) {
    for (size_t j = i + 1; j < objects.size(); ++j) {
      SceneObjectStrongPtr& a = objects[i];
      SceneObjectStrongPtr& b = objects[j];

      if (is_static_object(*a) || is_static_object(*b))
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

  while (accumulator >= Scene::FIXED_STEP) {
    step_simulation();
    accumulator -= Scene::FIXED_STEP;
  }
}
