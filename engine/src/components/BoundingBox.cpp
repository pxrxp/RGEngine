#include <engine/components/BoundingBox.h>
#include <algorithm>
#include <cmath>
#include <utility>

bool
BoundingBox::intersects_ray(const glm::vec3& origin,
                            const glm::vec3& dir,
                            float& t_out,
                            glm::vec3& hit_normal) const
{
  float tmin = 0.001f;
  float tmax = 1000.0f;
  glm::vec3 norm_min(0.0f);

  for (int i = 0; i < 3; ++i) {
    if (std::abs(dir[i]) < 1e-8f) {
      if (origin[i] < m_min[i] || origin[i] > m_max[i])
        return false;
    } else {
      float inv_d = 1.0f / dir[i];
      float t1 = (m_min[i] - origin[i]) * inv_d;
      float t2 = (m_max[i] - origin[i]) * inv_d;
      glm::vec3 n(0.0f);
      n[i] = -1.0f;

      if (t1 > t2) {
        std::swap(t1, t2);
        n[i] = 1.0f;
      }

      if (t1 > tmin) {
        tmin = t1;
        norm_min = n;
      }
      tmax = std::min(tmax, t2);

      if (tmin > tmax)
        return false;
    }
  }

  t_out = tmin;
  hit_normal = norm_min;
  return true;
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

#ifndef NDEBUG
std::ostream&
operator<<(std::ostream& os, const BoundingBox& bb)
{
  os << "BoundingBox(min=(" << bb.get_min().x << ", " << bb.get_min().y << ", "
     << bb.get_min().z << "), max=(" << bb.get_max().x << ", " << bb.get_max().y
     << ", " << bb.get_max().z << "))";
  return os;
}
#endif
