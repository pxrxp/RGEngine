#include "test_framework.h"

#include <engine/components/BoundingBox.h>

#include <glm/glm.hpp>

TEST_CASE(bounding_box_ray_hits_the_near_face)
{
  const BoundingBox box({ 0.0f, 0.0f, 0.0f }, { 2.0f, 2.0f, 2.0f });
  float distance = 0.0f;
  glm::vec3 normal(0.0f);

  CHECK(box.intersects_ray(glm::vec3(-1.0f, 1.0f, 1.0f),
                           glm::vec3(1.0f, 0.0f, 0.0f),
                           distance,
                           normal));
  CHECK_NEAR(distance, 1.0f, 1e-4);
  CHECK_NEAR(normal.x, -1.0f, 1e-4);
  CHECK_NEAR(normal.y, 0.0f, 1e-4);
  CHECK_NEAR(normal.z, 0.0f, 1e-4);
}

TEST_CASE(bounding_box_ray_misses_outside_the_slab)
{
  const BoundingBox box({ 0.0f, 0.0f, 0.0f }, { 2.0f, 2.0f, 2.0f });
  float distance = 123.0f;
  glm::vec3 normal(1.0f);

  CHECK(!box.intersects_ray(glm::vec3(-1.0f, 1.0f, 5.0f),
                            glm::vec3(1.0f, 0.0f, 0.0f),
                            distance,
                            normal));
  CHECK_NEAR(distance, 123.0f, 0.0);
}

TEST_CASE(bounding_box_ray_pointing_away_is_a_miss)
{
  const BoundingBox box({ 0.0f, 0.0f, 0.0f }, { 2.0f, 2.0f, 2.0f });
  float distance = 0.0f;
  glm::vec3 normal(0.0f);

  CHECK(!box.intersects_ray(glm::vec3(-5.0f, 1.0f, 1.0f),
                            glm::vec3(-1.0f, 0.0f, 0.0f),
                            distance,
                            normal));
}

TEST_CASE(bounding_box_ray_from_inside_starts_at_the_origin)
{
  const BoundingBox box({ 0.0f, 0.0f, 0.0f }, { 2.0f, 2.0f, 2.0f });
  float distance = 0.0f;
  glm::vec3 normal(0.0f);

  CHECK(
    box.intersects_ray(glm::vec3(1.0f, 1.0f, 1.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f),
                       distance,
                       normal));
  CHECK(distance > 0.0f);
  CHECK(distance < 2.0f);
}

TEST_CASE(bounding_box_add_point_grows_the_bounds)
{
  BoundingBox box({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f });
  box.add_point(glm::vec3(3.0f, 0.0f, 2.0f));

  CHECK_NEAR(box.get_min().x, 1.0f, 0.0);
  CHECK_NEAR(box.get_min().y, 0.0f, 0.0);
  CHECK_NEAR(box.get_min().z, 1.0f, 0.0);
  CHECK_NEAR(box.get_max().x, 3.0f, 0.0);
  CHECK_NEAR(box.get_max().y, 1.0f, 0.0);
  CHECK_NEAR(box.get_max().z, 2.0f, 0.0);
}

TEST_CASE(bounding_box_setters_mark_the_box_dirty)
{
  BoundingBox box({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
  box.set_dirty();
  CHECK(box.is_dirty());

  box.set_min(glm::vec3(-1.0f));
  CHECK(box.is_dirty());
  CHECK_NEAR(box.get_min().x, -1.0f, 0.0);

  box.set_max(glm::vec3(4.0f));
  CHECK(box.is_dirty());
  CHECK_NEAR(box.get_max().z, 4.0f, 0.0);
}
