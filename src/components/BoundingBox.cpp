#include "BoundingBox.h"

BoundingBox::
operator bool() const noexcept
{
  return m_min != m_max;
}

BoundingBox::BoundingBox(const std::vector<Vertex_Pos>& points)
{
  if (points.empty()) {
    m_min = glm::vec3(0.f);
    m_max = glm::vec3(0.f);
    return;
  }

  m_min = points[0].position;
  m_max = points[0].position;

  for (size_t i = 1; i < points.size(); ++i) {
    add_point(points[i].position);
  }
}

BoundingBox&
BoundingBox::resize(const std::vector<Vertex_Pos>& points)
{
  m_min = glm::vec3(0.f);
  m_max = glm::vec3(0.f);

  if (!points.empty()) {
    m_min = points[0].position;
    m_max = points[0].position;

    for (size_t i = 1; i < points.size(); ++i) {
      add_point(points[i].position);
    }
  }
  return *this;
}

Where
BoundingBox::where(const glm::vec3& point) const
{
  if ((point.x > m_min.x && point.x < m_max.x) &&
      (point.y > m_min.y && point.y < m_max.y) &&
      (point.z > m_min.z && point.z < m_max.z)) {
    return Inside;
  }

  if ((point.x < m_min.x || point.x > m_max.x) ||
      (point.y < m_min.y || point.y > m_max.y) ||
      (point.z < m_min.z || point.z > m_max.z)) {
    return Outside;
  }

  return AtBoundary;
}

bool
BoundingBox::intersects(const BoundingBox& other) const
{
  return (m_min.x <= other.m_max.x && m_max.x >= other.m_min.x) &&
         (m_min.y <= other.m_max.y && m_max.y >= other.m_min.y) &&
         (m_min.z <= other.m_max.z && m_max.z >= other.m_min.z);
}

BoundingBox&
BoundingBox::add_point(const glm::vec3& point)
{
  m_min.x = glm::min(m_min.x, point.x);
  m_min.y = glm::min(m_min.y, point.y);
  m_min.z = glm::min(m_min.z, point.z);

  m_max.x = glm::max(m_max.x, point.x);
  m_max.y = glm::max(m_max.y, point.y);
  m_max.z = glm::max(m_max.z, point.z);

  return *this;
}

std::vector<Vertex_Pos>
BoundingBox::corners() const
{
  return { { m_min.x, m_min.y, m_min.z }, { m_max.x, m_min.y, m_min.z },
           { m_min.x, m_max.y, m_min.z }, { m_max.x, m_max.y, m_min.z },
           { m_min.x, m_min.y, m_max.z }, { m_max.x, m_min.y, m_max.z },
           { m_min.x, m_max.y, m_max.z }, { m_max.x, m_max.y, m_max.z } };
}

std::vector<LineIndices>
BoundingBox::indices()
{
  return {
    { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 }, { 0, 2 }, { 1, 3 },
    { 4, 6 }, { 5, 7 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
  };
}

BoundingBox&
BoundingBox::set_min(const glm::vec3& min)
{
  m_min = min;
  set_dirty();
  return *this;
}
BoundingBox&
BoundingBox::set_max(const glm::vec3& max)
{
  m_max = max;
  set_dirty();
  return *this;
}

BoundingBox&
BoundingBox::offset(const glm::vec3& offset)
{
  m_delta_pos = offset;
  set_dirty();
  return *this;
}

BoundingBox&
BoundingBox::expand(const glm::vec3& expansion)
{
  m_delta_size = expansion;
  set_dirty();
  return *this;
}

#ifndef NDEBUG
std::ostream&
operator<<(std::ostream& os, const BoundingBox& bb)
{
  os << "BoundingBox(min=(" << bb.get_min().x << ", " << bb.get_min().y << ", "
     << bb.get_min().z << "), max=(" << bb.get_max().x << ", " << bb.get_max().y
     << ", " << bb.get_max().z << "), offset=(" << bb.get_offset().x << ", "
     << bb.get_offset().y << ", " << bb.get_offset().z << "), expansion=("
     << bb.get_expansion().x << ", " << bb.get_expansion().y << ", "
     << bb.get_expansion().z;
  return os;
}
#endif
