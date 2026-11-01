
#pragma once

#include <engine/components/BoundingBox.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace CollisionVoxelizer {
using Triangle = std::array<glm::vec3, 3>;

inline bool
triangle_intersects_box(const Triangle& triangle,
                        const glm::vec3& box_min,
                        const glm::vec3& box_max)
{
  // Translate triangle so the box is centered at the origin.
  const glm::vec3 center = (box_min + box_max) * 0.5f;
  const glm::vec3 half = (box_max - box_min) * 0.5f;

  const glm::vec3 v0 = triangle[0] - center;
  const glm::vec3 v1 = triangle[1] - center;
  const glm::vec3 v2 = triangle[2] - center;

  const glm::vec3 edges[] = { v1 - v0, v2 - v1, v0 - v2 };

  // Separating-axis test: box axes, triangle normal, and
  // each triangle edge crossed with each box axis.
  auto separated = [&](const glm::vec3& axis) -> bool {
    const float axis_len2 = glm::dot(axis, axis);

    if (axis_len2 < 1e-12f)
      return false;

    const float p0 = glm::dot(v0, axis);
    const float p1 = glm::dot(v1, axis);
    const float p2 = glm::dot(v2, axis);

    const float tri_min = std::min({ p0, p1, p2 });
    const float tri_max = std::max({ p0, p1, p2 });

    const float radius = half.x * std::abs(axis.x) + half.y * std::abs(axis.y) +
                         half.z * std::abs(axis.z);

    return tri_min > radius || tri_max < -radius;
  };

  // The three coordinate axes.
  if (separated(glm::vec3(1.f, 0.f, 0.f)) ||
      separated(glm::vec3(0.f, 1.f, 0.f)) ||
      separated(glm::vec3(0.f, 0.f, 1.f))) {
    return false;
  }

  // Triangle plane normal.
  if (separated(glm::cross(edges[0], edges[1])))
    return false;

  // Nine edge-cross-box-axis tests.
  const glm::vec3 axes[] = { { 1.f, 0.f, 0.f },
                             { 0.f, 1.f, 0.f },
                             { 0.f, 0.f, 1.f } };

  for (const auto& edge : edges) {
    for (const auto& axis : axes) {
      if (separated(glm::cross(edge, axis)))
        return false;
    }
  }

  return true;
}

inline bool
intersects_any_triangle(const glm::vec3& box_min,
                        const glm::vec3& box_max,
                        const std::vector<Triangle>& triangles)
{
  for (const Triangle& triangle : triangles) {
    if (triangle_intersects_box(triangle, box_min, box_max))
      return true;
  }

  return false;
}

inline void
subdivide(const glm::vec3& box_min,
          const glm::vec3& box_max,
          const std::vector<Triangle>& triangles,
          float min_cell_size,
          std::vector<BoundingBox>& output)
{
  // Shrink away empty space: reject cells that intersect no triangles.
  if (!intersects_any_triangle(box_min, box_max, triangles))
    return;

  const glm::vec3 size = box_max - box_min;
  const float largest_dimension = std::max({ size.x, size.y, size.z });

  // Stop once every dimension is at or below the requested resolution.
  if (largest_dimension <= min_cell_size) {
    output.emplace_back(box_min, box_max);
    return;
  }

  const glm::vec3 midpoint = (box_min + box_max) * 0.5f;

  // Split along the largest dimension. This avoids generating eight
  // children when only one axis needs further refinement.
  int axis = 0;
  if (size.y > size.x)
    axis = 1;
  if (size.z > size[axis])
    axis = 2;

  glm::vec3 left_max = box_max;
  glm::vec3 right_min = box_min;

  left_max[axis] = midpoint[axis];
  right_min[axis] = midpoint[axis];

  subdivide(box_min, left_max, triangles, min_cell_size, output);
  subdivide(right_min, box_max, triangles, min_cell_size, output);
}

inline std::vector<BoundingBox>
build(const glm::vec3& bounds_min,
      const glm::vec3& bounds_max,
      const std::vector<Triangle>& triangles,
      float min_cell_size)
{
  std::vector<BoundingBox> result;

  if (triangles.empty() || min_cell_size <= 0.f)
    return result;

  subdivide(bounds_min, bounds_max, triangles, min_cell_size, result);

  return result;
}
} // namespace CollisionVoxelizer
