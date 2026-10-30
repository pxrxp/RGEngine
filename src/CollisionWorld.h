#pragma once

#include "TriangleMeshCollider.h"
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

// Scene-level registry for static triangle-mesh collision geometry.
// Scene/game code registers geometry; movement controllers and ray queries use
// this interface without needing to know how colliders are stored or
// accelerated. Dynamic rigid bodies and primitive colliders can be added to
// this world later without changing the triangle-mesh query API.
class CollisionWorld
{
public:
  using Triangle = TriangleMeshCollider::Triangle;

private:
  std::vector<TriangleMeshCollider> m_static_meshes;

public:
  void clear() { m_static_meshes.clear(); }

  bool empty() const { return m_static_meshes.empty(); }
  std::size_t static_mesh_count() const { return m_static_meshes.size(); }

  // Registers a static mesh by copying/transformation-building its collision
  // triangles into a BVH. The source mesh and scene object remain independent.
  std::size_t add_static_mesh(const std::vector<Triangle>& triangles,
                              const glm::mat4& world_transform)
  {
    TriangleMeshCollider collider;
    collider.build(triangles, world_transform);
    const std::size_t count = collider.triangle_count();
    if (!collider.empty())
      m_static_meshes.push_back(std::move(collider));
    return count;
  }

  bool blocks_vertical_capsule(float x,
                               float z,
                               float from_x,
                               float from_z,
                               float feet_y,
                               float head_y,
                               float radius) const
  {
    for (const auto& collider : m_static_meshes) {
      if (collider.blocks_vertical_capsule(
            x, z, from_x, from_z, feet_y, head_y, radius))
        return true;
    }
    return false;
  }

  bool raycast(const glm::vec3& origin,
               const glm::vec3& direction,
               float max_distance,
               float& out_distance) const
  {
    bool hit = false;
    float best = max_distance;
    for (const auto& collider : m_static_meshes) {
      float distance = 0.0f;
      if (collider.raycast(origin, direction, best, distance) &&
          distance < best) {
        best = distance;
        hit = true;
      }
    }
    if (hit)
      out_distance = best;
    return hit;
  }

  bool find_landing_surface(float x,
                            float z,
                            float radius,
                            float previous_feet_y,
                            float feet_y,
                            float minimum_up_normal,
                            float& out_y) const
  {
    bool found = false;
    float highest = -std::numeric_limits<float>::infinity();
    for (const auto& collider : m_static_meshes) {
      float surface_y = 0.0f;
      if (collider.find_landing_surface(x,
                                        z,
                                        radius,
                                        previous_feet_y,
                                        feet_y,
                                        minimum_up_normal,
                                        surface_y) &&
          surface_y > highest) {
        highest = surface_y;
        found = true;
      }
    }
    if (found)
      out_y = highest;
    return found;
  }

  bool find_ground_height(float x,
                          float z,
                          float max_y,
                          float min_y,
                          float& out_y) const
  {
    bool found = false;
    float highest = min_y;
    for (const auto& collider : m_static_meshes) {
      float surface_y = 0.0f;
      if (collider.find_ground_height(x, z, max_y, min_y, surface_y) &&
          surface_y > highest) {
        highest = surface_y;
        found = true;
      }
    }
    if (found)
      out_y = highest;
    return found;
  }
};
