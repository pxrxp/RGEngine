#include "BoundingBox.h"

BoundingBox::operator bool() const noexcept
{
  return min != max;
}

BoundingBox::BoundingBox(const std::vector<Vertex_Pos>& points)
{
  if (points.empty()) {
    min = glm::vec3(0.f);
    max = glm::vec3(0.f);
    return;
  }

  min = points[0].position;
  max = points[0].position;

  for (size_t i = 1; i < points.size(); ++i) {
    add_point(points[i].position);
  }
}

Where
BoundingBox::where(const glm::vec3& point) const
{
  if ((point.x > min.x && point.x < max.x) &&
      (point.y > min.y && point.y < max.y) &&
      (point.z > min.z && point.z < max.z)) {
    return Inside;
  }

  if ((point.x < min.x || point.x > max.x) ||
      (point.y < min.y || point.y > max.y) ||
      (point.z < min.z || point.z > max.z)) {
    return Outside;
  }

  return AtBoundary;
}

bool
BoundingBox::intersects(const BoundingBox& other) const
{
  return (min.x <= other.max.x && max.x >= other.min.x) &&
         (min.y <= other.max.y && max.y >= other.min.y) &&
         (min.z <= other.max.z && max.z >= other.min.z);
}

BoundingBox&
BoundingBox::add_point(const glm::vec3& point)
{
  min.x = glm::min(min.x, point.x);
  min.y = glm::min(min.y, point.y);
  min.z = glm::min(min.z, point.z);

  max.x = glm::max(max.x, point.x);
  max.y = glm::max(max.y, point.y);
  max.z = glm::max(max.z, point.z);

  return *this;
}

std::vector<Vertex_Pos>
BoundingBox::corners() const
{
  return { { min.x, min.y, min.z }, { max.x, min.y, min.z },
           { min.x, max.y, min.z }, { max.x, max.y, min.z },
           { min.x, min.y, max.z }, { max.x, min.y, max.z },
           { min.x, max.y, max.z }, { max.x, max.y, max.z } };
}

std::vector<LineIndices>
BoundingBox::indices()
{
  return {
    { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 }, { 0, 2 }, { 1, 3 },
    { 4, 6 }, { 5, 7 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
  };
}

#ifndef NDEBUG
std::ostream&
operator<<(std::ostream& os, const BoundingBox& bb)
{
  os << "BoundingBox(min=(" << bb.min.x << ", " << bb.min.y << ", " << bb.min.z
     << "), max=(" << bb.max.x << ", " << bb.max.y << ", " << bb.max.z << "))";
  return os;
}
#endif
