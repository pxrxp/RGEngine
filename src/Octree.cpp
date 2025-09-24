#include "Scene.h"
#include "components/BoundingBox.h"
#include <array>
#include <glm/glm.hpp>
#include <memory>

#include "debug.h"

class OctreeNode
{
private:
  std::array<std::shared_ptr<OctreeNode>, 8> m_children;
  std::vector<SceneObjectPtr> m_scenes;
  BoundingBox m_boundary;

public:
  BoundingBox get_boundary() const;
  static void get(const glm::vec3& point,
                  const std::shared_ptr<OctreeNode>& root,
                  std::vector<std::shared_ptr<OctreeNode>>& result);
  static void get(const BoundingBox& box,
                  const std::shared_ptr<OctreeNode>& root,
                  std::vector<std::shared_ptr<OctreeNode>>& result);
};

BoundingBox
OctreeNode::get_boundary() const
{
  return m_boundary;
}

void
OctreeNode::get(const glm::vec3& point,
                const std::shared_ptr<OctreeNode>& root,
                std::vector<std::shared_ptr<OctreeNode>>& result)
{
  if (root->m_boundary.where(point) == Inside) {
    result.push_back(root);
    return;
  }

  if (root->m_boundary.where(point) == AtBoundary) {
    result.push_back(root);
  }

  for (const auto& child : root->m_children) {
    get(point, child, result);
  }
}

void
OctreeNode::get(const BoundingBox& box,
                const std::shared_ptr<OctreeNode>& root,
                std::vector<std::shared_ptr<OctreeNode>>& result)
{
  if (root->m_boundary.collides(box)) {
    result.push_back(root);
  }

  for (const auto& child : root->m_children) {
    get(box, child, result);
  }
}
