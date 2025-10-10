#pragma once
#include "TransformComponent.h"
#include "vertex_formats.h"
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

enum Where
{
  Outside,
  Inside,
  AtBoundary
};

struct BoundingBox
{
private:
  glm::vec3 m_delta_pos = glm::vec3(0.f);
  glm::vec3 m_delta_size = glm::vec3(0.f);

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

  explicit BoundingBox(const std::vector<Vertex_Pos>& points);
  explicit operator bool() const noexcept;

  Where where(const glm::vec3& point) const;
  bool intersects(const BoundingBox& other) const;

  BoundingBox& add_point(const glm::vec3& point);
  BoundingBox& resize(const std::vector<Vertex_Pos>& points);

  std::vector<Vertex_Pos> corners() const;
  static std::vector<LineIndices> indices();

  bool is_dirty() const { return m_dirty; }
  void set_dirty() { m_dirty = true; }

  const glm::vec3& get_min() const { return m_min; }
  const glm::vec3& get_max() const { return m_max; }

  const glm::vec3& get_offset() const { return m_delta_pos; }
  const glm::vec3& get_expansion() const { return m_delta_size; }

  BoundingBox& set_min(const glm::vec3& min);
  BoundingBox& set_max(const glm::vec3& max);
  BoundingBox& offset(const glm::vec3& offset);
  BoundingBox& expand(const glm::vec3& expansion);

  friend class SceneObject;
};

#ifndef NDEBUG
std::ostream&
operator<<(std::ostream& os, const BoundingBox& bb);
#endif
