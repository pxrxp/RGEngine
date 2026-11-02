#include "test_framework.h"

#include <engine/TriangleMeshCollider.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace {

// A two-triangle, upward facing quad covering x/z in [0, 2] at y = 0.
std::vector<TriangleMeshCollider::Triangle>
unit_quad()
{
  return {
    { glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(2.0f, 0.0f, 2.0f),
      glm::vec3(2.0f, 0.0f, 0.0f) },
    { glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 0.0f, 2.0f),
      glm::vec3(2.0f, 0.0f, 2.0f) },
  };
}

} // namespace

TEST_CASE(collider_builds_only_valid_triangles)
{
  TriangleMeshCollider collider;
  CHECK(collider.empty());

  collider.build(unit_quad(), glm::mat4(1.0f));
  CHECK(!collider.empty());
  CHECK(collider.triangle_count() == 2);

  std::vector<TriangleMeshCollider::Triangle> degenerate = {
    { glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(2.0f, 0.0f, 0.0f) },
  };
  TriangleMeshCollider flat;
  flat.build(degenerate, glm::mat4(1.0f));
  CHECK(flat.empty());
}

TEST_CASE(collider_raycast_hits_from_above)
{
  TriangleMeshCollider collider;
  collider.build(unit_quad(), glm::mat4(1.0f));

  float distance = 0.0f;
  CHECK(collider.raycast(
    glm::vec3(1.0f, 5.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), 10.0f, distance));
  CHECK_NEAR(distance, 5.0f, 1e-3);
}

TEST_CASE(collider_raycast_respects_the_distance_budget)
{
  TriangleMeshCollider collider;
  collider.build(unit_quad(), glm::mat4(1.0f));

  float distance = 77.0f;
  CHECK(!collider.raycast(
    glm::vec3(1.0f, 5.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), 4.0f, distance));
  CHECK_NEAR(distance, 77.0f, 0.0);
}

TEST_CASE(collider_raycast_misses_beside_the_quad)
{
  TriangleMeshCollider collider;
  collider.build(unit_quad(), glm::mat4(1.0f));

  float distance = 0.0f;
  CHECK(!collider.raycast(
    glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), 10.0f, distance));
  CHECK(!collider.raycast(
    glm::vec3(1.0f, 5.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 10.0f, distance));
}

TEST_CASE(collider_raycast_uses_the_world_transform)
{
  TriangleMeshCollider collider;
  collider.build(unit_quad(),
                 glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 0.0f)));

  float distance = 0.0f;
  CHECK(collider.raycast(
    glm::vec3(1.0f, 8.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), 10.0f, distance));
  CHECK_NEAR(distance, 5.0f, 1e-3);
}

TEST_CASE(collider_finds_the_highest_walkable_surface)
{
  TriangleMeshCollider collider;
  collider.build(unit_quad(), glm::mat4(1.0f));

  float height = -5.0f;
  CHECK(collider.find_ground_height(1.0f, 1.0f, 5.0f, -1.0f, height));
  CHECK_NEAR(height, 0.0f, 1e-3);

  CHECK(!collider.find_ground_height(10.0f, 10.0f, 5.0f, -1.0f, height));
}

TEST_CASE(collider_ground_search_ignores_ceiling_height)
{
  TriangleMeshCollider collider;
  collider.build(unit_quad(), glm::mat4(1.0f));

  float height = 0.0f;
  CHECK(collider.find_ground_height(1.0f, 1.0f, 5.0f, 2.0f, height) == false);
}

TEST_CASE(collider_finds_landing_surface_while_descending)
{
  TriangleMeshCollider collider;
  collider.build(unit_quad(), glm::mat4(1.0f));

  float height = 42.0f;
  CHECK(collider.find_landing_surface(1.0f, 1.0f, 0.4f, 1.0f, -0.1f, 0.45f, height));
  CHECK_NEAR(height, 0.0f, 1e-3);

  CHECK(!collider.find_landing_surface(
    1.0f, 1.0f, 0.4f, 4.0f, 2.0f, 0.45f, height));
}

TEST_CASE(collider_capsule_blocks_inward_motion_but_not_escape)
{
  TriangleMeshCollider collider;
  collider.build(unit_quad(), glm::mat4(1.0f));

  // Walking from outside the quad into it must be blocked.
  CHECK(
    collider.blocks_vertical_capsule(1.0f, 1.0f, 1.0f, 5.0f, -0.2f, 1.8f, 0.4f));
  // Walking away from the quad is always allowed.
  CHECK(!collider.blocks_vertical_capsule(
    1.0f, 5.0f, 1.0f, 1.0f, -0.2f, 1.8f, 0.4f));
  // A capsule that clears the surface is not blocked.
  CHECK(!collider.blocks_vertical_capsule(
    1.0f, 1.0f, 1.0f, 5.0f, 3.0f, 4.8f, 0.4f));
  // A capsule resting exactly on the surface can still move.
  CHECK(!collider.blocks_vertical_capsule(
    1.0f, 1.0f, 1.0f, 5.0f, 0.0f, 1.8f, 0.4f));
}

TEST_CASE(collider_query_returns_overlapping_triangles)
{
  TriangleMeshCollider collider;
  collider.build(unit_quad(), glm::mat4(1.0f));

  std::vector<std::size_t> hits;
  collider.query(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(3.0f, 1.0f, 3.0f), hits);
  CHECK(hits.size() == 2);

  collider.query(glm::vec3(10.0f), glm::vec3(20.0f), hits);
  CHECK(hits.empty());
}

TEST_CASE(collider_height_and_closest_point_helpers)
{
  const TriangleMeshCollider::Triangle triangle = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(2.0f, 0.0f, 2.0f),
    glm::vec3(2.0f, 0.0f, 0.0f),
  };

  float height = 0.0f;
  CHECK(TriangleMeshCollider::height_at_xz(triangle, 1.5f, 0.5f, height));
  CHECK_NEAR(height, 0.0f, 1e-4);
  CHECK(!TriangleMeshCollider::height_at_xz(triangle, 10.0f, 10.0f, height));

  const glm::vec3 closest =
    TriangleMeshCollider::closest_point(glm::vec3(1.5f, 2.0f, 0.5f), triangle);
  CHECK_NEAR(closest.x, 1.5f, 1e-3);
  CHECK_NEAR(closest.y, 0.0f, 1e-3);
  CHECK_NEAR(closest.z, 0.5f, 1e-3);
}
