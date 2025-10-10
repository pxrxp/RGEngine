#include "Scene.h"

void
resolve_penetration(SceneObject& a, SceneObject& b)
{
  BoundingBox& aabb_a = a.get_world_AABB();
  BoundingBox& aabb_b = b.get_world_AABB();

  float overlap_x = std::min(aabb_a.get_max().x, aabb_b.get_max().x) -
                    std::max(aabb_a.get_min().x, aabb_b.get_min().x);
  float overlap_y = std::min(aabb_a.get_max().y, aabb_b.get_max().y) -
                    std::max(aabb_a.get_min().y, aabb_b.get_min().y);
  float overlap_z = std::min(aabb_a.get_max().z, aabb_b.get_max().z) -
                    std::max(aabb_a.get_min().z, aabb_b.get_min().z);

  glm::vec3 mtv;

  if (overlap_x < overlap_y && overlap_x < overlap_z) {
    mtv = glm::vec3(overlap_x, 0, 0);
  } else if (overlap_y < overlap_z) {
    mtv = glm::vec3(0, overlap_y, 0);
  } else {
    mtv = glm::vec3(0, 0, overlap_z);
  }

  glm::vec3 direction = b.get_local_transform().get_position() -
                        a.get_local_transform().get_position();
  if (glm::dot(direction, mtv) < 0.0f) {
    mtv = -mtv;
  }

  if (mtv.x != 0)
    mtv.x -= 0.001f;
  if (mtv.y != 0)
    mtv.y -= 0.001f;
  if (mtv.y != 0)
    mtv.y -= 0.001f;

  float move_a = 0.f, move_b = 0.f, div = 0.f;
  float mass_a = a.get_physics_component().get_mass();
  float mass_b = b.get_physics_component().get_mass();

  if (std::isfinite(mass_a) && mass_a > 0.0001f) {
    move_a = mass_a;
    div += mass_a;
  }

  if (std::isfinite(mass_b) && mass_b > 0.0001f) {
    move_b = mass_b;
    div += mass_b;
  }

  if (div > 0.0001f) {
    move_a /= div;
    move_b /= div;
  }

  a.get_local_transform().translate(-mtv * move_a);
  b.get_local_transform().translate(mtv * move_b);
}

void
resolve_velocity(SceneObject& a, SceneObject& b)
{
  // glm::vec3 center_A = a.get_local_transform().get_position();
  // glm::vec3 center_B = b.get_local_transform().get_position();

  // glm::vec3 direction_vector = center_B - center_A;
  // glm::vec3 collision_normal = glm::normalize(direction_vector);

  // glm::vec3 v_rel = b.get_physics_component().get_velocity() -
  // a.get_physics_component().get_velocity(); float vel_along_normal =
  // glm::dot(v_rel, collision_normal);

  // if (vel_along_normal > 0)
  //   return;

  // float e = std::min(a.get_physics_component().get_restitution(),
  // b.get_physics_component().get_restitution()); float j = -(1 + e) *
  // vel_along_normal; j /= (1 / a.get_physics_component().get_mass()) + (1 /
  // b.get_physics_component().get_mass());

  // glm::vec3 impulse_vec = j * collision_normal;
  // a.get_physics_component().apply_linear_impulse(-impulse_vec);
  // b.get_physics_component().apply_linear_impulse(impulse_vec);
}

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
    object->get_physics_component().integrate_position();
  }

  for (size_t i = 0; i < objects.size(); ++i) {
    for (size_t j = i + 1; j < objects.size(); ++j) {
      SceneObjectStrongPtr& a = objects[i];
      SceneObjectStrongPtr& b = objects[j];

      const BoundingBox& aabb_a = a->get_world_AABB();
      const BoundingBox& aabb_b = b->get_world_AABB();

      if (aabb_a && aabb_b && aabb_a.intersects(aabb_b)) {
        resolve_penetration(*a, *b);
        resolve_velocity(*a, *b);
      }
    }
  }

  for (auto& object : objects) {
    auto& physics = object->get_physics_component();
    physics.clear_forces();

    if (physics.has_gravity() &&
        object->get_local_transform().get_position().y > 0) {
      glm::vec3 gravity_force =
        glm::vec3(0.0f, -9.8f, 0.0f) * physics.get_mass();
      physics.apply_force(gravity_force);
    }

    auto velocity = physics.get_velocity();
    if (std::isfinite(velocity.x) && std::isfinite(velocity.y) &&
        std::isfinite(velocity.z))
      physics.apply_force(-physics.get_drag() * physics.get_velocity());
  }

  for (auto& object : objects) {
    object->get_physics_component().integrate_velocity();
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
