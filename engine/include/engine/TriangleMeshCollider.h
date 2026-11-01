#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <glm/glm.hpp>
#include <limits>
#include <numeric>
#include <vector>

// Reusable static triangle-mesh collider backed by a bounding-volume hierarchy
// (BVH). Accepts geometry and a transform from the caller; it has no game- or
// asset-specific logic.
class TriangleMeshCollider
{
public:
  using Triangle = std::array<glm::vec3, 3>;

private:
  struct TriData
  {
    Triangle p{};
    glm::vec3 mn{ 0.0f };
    glm::vec3 mx{ 0.0f };
    glm::vec3 center{ 0.0f };
  };

  struct Node
  {
    glm::vec3 mn{ 0.0f };
    glm::vec3 mx{ 0.0f };
    std::size_t begin = 0;
    std::size_t count = 0;
    int left = -1;
    int right = -1;
    bool leaf() const { return left < 0; }
  };

  std::vector<TriData> m_triangles;
  std::vector<Node> m_nodes;

  static bool overlaps(const glm::vec3& amin,
                       const glm::vec3& amax,
                       const glm::vec3& bmin,
                       const glm::vec3& bmax)
  {
    return amin.x <= bmax.x && amax.x >= bmin.x && amin.y <= bmax.y &&
           amax.y >= bmin.y && amin.z <= bmax.z && amax.z >= bmin.z;
  }

  int build_node(std::size_t begin, std::size_t end)
  {
    Node node;
    node.mn = glm::vec3(std::numeric_limits<float>::max());
    node.mx = glm::vec3(std::numeric_limits<float>::lowest());
    glm::vec3 centroid_min(std::numeric_limits<float>::max());
    glm::vec3 centroid_max(std::numeric_limits<float>::lowest());
    for (std::size_t i = begin; i < end; ++i) {
      node.mn = glm::min(node.mn, m_triangles[i].mn);
      node.mx = glm::max(node.mx, m_triangles[i].mx);
      centroid_min = glm::min(centroid_min, m_triangles[i].center);
      centroid_max = glm::max(centroid_max, m_triangles[i].center);
    }

    const int node_index = static_cast<int>(m_nodes.size());
    m_nodes.push_back(node);
    const std::size_t count = end - begin;
    if (count <= 12) {
      m_nodes[node_index].begin = begin;
      m_nodes[node_index].count = count;
      return node_index;
    }

    const glm::vec3 extent = centroid_max - centroid_min;
    int axis = extent.y > extent.x ? 1 : 0;
    if (extent.z > extent[axis])
      axis = 2;
    const std::size_t mid = begin + count / 2;
    std::nth_element(m_triangles.begin() + static_cast<std::ptrdiff_t>(begin),
                     m_triangles.begin() + static_cast<std::ptrdiff_t>(mid),
                     m_triangles.begin() + static_cast<std::ptrdiff_t>(end),
                     [axis](const TriData& a, const TriData& b) {
                       return a.center[axis] < b.center[axis];
                     });
    const int left = build_node(begin, mid);
    const int right = build_node(mid, end);
    m_nodes[node_index].left = left;
    m_nodes[node_index].right = right;
    return node_index;
  }

  void query_node(int node_index,
                  const glm::vec3& mn,
                  const glm::vec3& mx,
                  std::vector<std::size_t>& out) const
  {
    if (node_index < 0)
      return;
    const Node& node = m_nodes[static_cast<std::size_t>(node_index)];
    if (!overlaps(node.mn, node.mx, mn, mx))
      return;
    if (node.leaf()) {
      for (std::size_t i = node.begin; i < node.begin + node.count; ++i) {
        if (overlaps(m_triangles[i].mn, m_triangles[i].mx, mn, mx))
          out.push_back(i);
      }
      return;
    }
    query_node(node.left, mn, mx, out);
    query_node(node.right, mn, mx, out);
  }

public:
  void build(const std::vector<Triangle>& local_triangles,
             const glm::mat4& world)
  {
    m_triangles.clear();
    m_nodes.clear();
    m_triangles.reserve(local_triangles.size());
    for (const Triangle& local : local_triangles) {
      TriData tri;
      tri.mn = glm::vec3(std::numeric_limits<float>::max());
      tri.mx = glm::vec3(std::numeric_limits<float>::lowest());
      for (int i = 0; i < 3; ++i) {
        const glm::vec4 transformed = world * glm::vec4(local[i], 1.0f);
        tri.p[i] = glm::vec3(transformed) /
                   (std::abs(transformed.w) > 1e-7f ? transformed.w : 1.0f);
        tri.mn = glm::min(tri.mn, tri.p[i]);
        tri.mx = glm::max(tri.mx, tri.p[i]);
      }
      tri.center = (tri.p[0] + tri.p[1] + tri.p[2]) / 3.0f;
      if (glm::dot(glm::cross(tri.p[1] - tri.p[0], tri.p[2] - tri.p[0]),
                   glm::cross(tri.p[1] - tri.p[0], tri.p[2] - tri.p[0])) >
          1e-12f)
        m_triangles.push_back(tri);
    }
    if (!m_triangles.empty()) {
      m_nodes.reserve(m_triangles.size() / 6 + 1);
      build_node(0, m_triangles.size());
    }
  }

  bool empty() const { return m_triangles.empty(); }
  std::size_t triangle_count() const { return m_triangles.size(); }
  const Triangle& triangle(std::size_t i) const { return m_triangles[i].p; }
  const glm::vec3& triangle_min(std::size_t i) const
  {
    return m_triangles[i].mn;
  }
  const glm::vec3& triangle_max(std::size_t i) const
  {
    return m_triangles[i].mx;
  }

  void query(const glm::vec3& mn,
             const glm::vec3& mx,
             std::vector<std::size_t>& out) const
  {
    out.clear();
    if (!m_nodes.empty())
      query_node(0, mn, mx, out);
  }

  bool find_ground_height(float x,
                          float z,
                          float max_y,
                          float min_y,
                          float& out_y) const
  {
    if (empty() || max_y <= min_y)
      return false;
    const float radius = 0.40f;
    const glm::vec3 qmin(x - radius, min_y - 0.1f, z - radius);
    const glm::vec3 qmax(x + radius, max_y + 0.1f, z + radius);
    std::vector<std::size_t> candidates;
    query(qmin, qmax, candidates);
    if (candidates.empty())
      return false;

    const float offsets[5][2] = { { 0.0f, 0.0f },
                                  { radius * 0.5f, 0.0f },
                                  { -radius * 0.5f, 0.0f },
                                  { 0.0f, radius * 0.5f },
                                  { 0.0f, -radius * 0.5f } };

    bool found = false;
    float highest = min_y;

    for (std::size_t index : candidates) {
      const Triangle& tri = triangle(index);
      const glm::vec3 n = glm::cross(tri[1] - tri[0], tri[2] - tri[0]);
      if (n.y <= 0.0f || n.y * n.y < 0.25f * glm::dot(n, n))
        continue;

      for (const auto& off : offsets) {
        float y = 0.0f;
        if (height_at_xz(tri, x + off[0], z + off[1], y)) {
          if (y <= max_y + 0.1f && y >= min_y - 0.1f && y > highest) {
            highest = y;
            found = true;
          }
        }
      }
    }

    if (found)
      out_y = highest;
    return found;
  }

  static glm::vec3 closest_point(const glm::vec3& p, const Triangle& t)
  {
    const glm::vec3 a = t[0], b = t[1], c = t[2];
    const glm::vec3 ab = b - a, ac = c - a, ap = p - a;
    const float d1 = glm::dot(ab, ap), d2 = glm::dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f)
      return a;
    const glm::vec3 bp = p - b;
    const float d3 = glm::dot(ab, bp), d4 = glm::dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3)
      return b;
    const float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
      return a + (d1 / (d1 - d3)) * ab;
    const glm::vec3 cp = p - c;
    const float d5 = glm::dot(ab, cp), d6 = glm::dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6)
      return c;
    const float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
      return a + (d2 / (d2 - d6)) * ac;
    const float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
      return b + ((d4 - d3) / ((d4 - d3) + (d5 - d6))) * (c - b);
    const float denom = 1.0f / (va + vb + vc);
    return a + ab * (vb * denom) + ac * (vc * denom);
  }

  // Tests a vertical capsule approximation using three sphere-center samples.
  // Dimensions and movement policy are supplied by the caller, not hardcoded
  // here.
  bool blocks_vertical_capsule(float x,
                               float z,
                               float from_x,
                               float from_z,
                               float feet_y,
                               float head_y,
                               float radius) const
  {
    if (empty() || radius <= 0.0f || head_y <= feet_y)
      return false;
    const glm::vec3 query_min(x - radius, feet_y + 0.001f, z - radius);
    const glm::vec3 query_max(x + radius, head_y - 0.001f, z + radius);
    std::vector<std::size_t> candidates;
    query(query_min, query_max, candidates);
    if (candidates.empty())
      return false;

    const float sample_y[3] = { feet_y + radius,
                                (feet_y + head_y) * 0.5f,
                                head_y - radius };
    float next_dist_sq = std::numeric_limits<float>::max();
    float current_dist_sq = std::numeric_limits<float>::max();
    for (std::size_t index : candidates) {
      const Triangle& tri = triangle(index);
      for (float y : sample_y) {
        const glm::vec3 next_point(x, y, z);
        const glm::vec3 next_closest = closest_point(next_point, tri);
        const glm::vec3 next_delta = next_point - next_closest;
        next_dist_sq = std::min(next_dist_sq, glm::dot(next_delta, next_delta));
        const glm::vec3 current_point(from_x, y, from_z);
        const glm::vec3 current_closest = closest_point(current_point, tri);
        const glm::vec3 current_delta = current_point - current_closest;
        current_dist_sq =
          std::min(current_dist_sq, glm::dot(current_delta, current_delta));
      }
    }
    const float radius_sq = radius * radius;
    if (next_dist_sq >= radius_sq)
      return false;
    // Permit movement out of existing penetration rather than trapping an
    // actor.
    return !(current_dist_sq < radius_sq &&
             next_dist_sq > current_dist_sq + 1e-5f);
  }

  // Ray direction is expected to be normalized; out_distance is unchanged on
  // miss.
  bool raycast(const glm::vec3& origin,
               const glm::vec3& direction,
               float max_distance,
               float& out_distance) const
  {
    if (empty() || max_distance <= 0.0f)
      return false;
    const glm::vec3 end = origin + direction * max_distance;
    const glm::vec3 mn = glm::min(origin, end) - glm::vec3(0.001f);
    const glm::vec3 mx = glm::max(origin, end) + glm::vec3(0.001f);
    std::vector<std::size_t> candidates;
    query(mn, mx, candidates);
    bool hit = false;
    float best = max_distance;
    for (std::size_t index : candidates) {
      const Triangle& tri = triangle(index);
      const glm::vec3 e1 = tri[1] - tri[0];
      const glm::vec3 e2 = tri[2] - tri[0];
      const glm::vec3 h = glm::cross(direction, e2);
      const float a = glm::dot(e1, h);
      if (std::abs(a) < 1e-7f)
        continue;
      const float f = 1.0f / a;
      const glm::vec3 offset = origin - tri[0];
      const float u = f * glm::dot(offset, h);
      if (u < 0.0f || u > 1.0f)
        continue;
      const glm::vec3 q = glm::cross(offset, e1);
      const float v = f * glm::dot(direction, q);
      if (v < 0.0f || u + v > 1.0f)
        continue;
      const float distance = f * glm::dot(e2, q);
      if (distance > 0.001f && distance < best) {
        best = distance;
        hit = true;
      }
    }
    if (hit)
      out_distance = best;
    return hit;
  }

  // Returns the highest walkable triangle surface crossed while descending.
  bool find_landing_surface(float x,
                            float z,
                            float radius,
                            float previous_feet_y,
                            float feet_y,
                            float minimum_up_normal,
                            float& out_y) const
  {
    if (empty() || radius < 0.0f || previous_feet_y < feet_y)
      return false;
    const glm::vec3 qmin(x - radius, feet_y - 0.05f, z - radius);
    const glm::vec3 qmax(x + radius, previous_feet_y + 0.05f, z + radius);
    std::vector<std::size_t> candidates;
    query(qmin, qmax, candidates);
    const float offsets[5][2] = { { 0, 0 },
                                  { radius * 0.65f, 0 },
                                  { -radius * 0.65f, 0 },
                                  { 0, radius * 0.65f },
                                  { 0, -radius * 0.65f } };
    bool found = false;
    float best = -std::numeric_limits<float>::infinity();
    for (std::size_t index : candidates) {
      const Triangle& tri = triangle(index);
      const glm::vec3 n = glm::cross(tri[1] - tri[0], tri[2] - tri[0]);
      const float n_len_sq = glm::dot(n, n);
      if (n_len_sq < 1e-10f ||
          std::abs(n.y) / std::sqrt(n_len_sq) < minimum_up_normal)
        continue;
      for (const auto& offset : offsets) {
        float surface_y = 0.0f;
        if (!height_at_xz(tri, x + offset[0], z + offset[1], surface_y))
          continue;
        if (previous_feet_y >= surface_y - 0.025f &&
            feet_y <= surface_y + 0.025f && surface_y > best) {
          best = surface_y;
          found = true;
        }
      }
    }
    if (found)
      out_y = best;
    return found;
  }

  static bool height_at_xz(const Triangle& t, float x, float z, float& y)
  {
    const glm::vec3 normal = glm::cross(t[1] - t[0], t[2] - t[0]);
    if (std::abs(normal.y) < 1e-5f)
      return false;
    const float u =
      ((t[1].z - t[2].z) * (x - t[2].x) + (t[2].x - t[1].x) * (z - t[2].z)) /
      ((t[1].z - t[2].z) * (t[0].x - t[2].x) +
       (t[2].x - t[1].x) * (t[0].z - t[2].z));
    const float v =
      ((t[2].z - t[0].z) * (x - t[2].x) + (t[0].x - t[2].x) * (z - t[2].z)) /
      ((t[1].z - t[2].z) * (t[0].x - t[2].x) +
       (t[2].x - t[1].x) * (t[0].z - t[2].z));
    const float w = 1.0f - u - v;
    if (u < -1e-4f || v < -1e-4f || w < -1e-4f)
      return false;
    y = u * t[0].y + v * t[1].y + w * t[2].y;
    return true;
  }
};
