#pragma once
#include <glm/glm.hpp>
#include <iostream>

struct BoundingBox
{
private:
  glm::vec3 m_min = glm::vec3(0.f);
  glm::vec3 m_max = glm::vec3(0.f);

  mutable bool m_dirty = true;

public:
  BoundingBox()
    : m_min(glm::vec3(0.f))
    , m_max(glm::vec3(0.f))
  {
  }

  BoundingBox(const glm::vec3& min, const glm::vec3& max)
    : m_min(min)
    , m_max(max)
  {
  }

  bool intersects_ray(const glm::vec3& origin,
                      const glm::vec3& dir,
                      float& t_out,
                      glm::vec3& hit_normal) const;

  BoundingBox& add_point(const glm::vec3& point);

  bool is_dirty() const { return m_dirty; }
  void set_dirty() { m_dirty = true; }

  const glm::vec3& get_min() const { return m_min; }
  const glm::vec3& get_max() const { return m_max; }

  BoundingBox& set_min(const glm::vec3& min);
  BoundingBox& set_max(const glm::vec3& max);

  friend class SceneObject;
};

#ifndef NDEBUG
std::ostream&
operator<<(std::ostream& os, const BoundingBox& bb);
#endif
