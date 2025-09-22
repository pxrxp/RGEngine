#include "Scene.h"
#include <array>
#include <unordered_set>
#include <glm/glm.hpp>
#include <memory>

enum Where { Outside, Inside, AtBoundary };

class Boundary {
  glm::vec3 min, max;

  Where where(glm::vec3 point) {
    bool at_x_boundary = point.x == min.x || point.x == max.x;
    bool at_y_boundary = point.y == min.y || point.y == max.y;
    bool at_z_boundary = point.z == min.z || point.z == max.z;

    bool at_boundary = at_x_boundary || at_y_boundary || at_z_boundary;

    if (at_boundary)
      return AtBoundary;

    bool inside_x_boundary = point.x > min.x && point.x < max.x;
    bool inside_y_boundary = point.y > min.y && point.y < max.y;
    bool inside_z_boundary = point.z > min.z && point.z < max.z;

    bool inside = inside_x_boundary && inside_y_boundary && inside_z_boundary;

    if (inside)
      return Inside;

    return Outside;
  }
};

class OctreeNode {
private:
  Boundary boundary;
  std::array<std::unique_ptr<OctreeNode>, 8> children;
  std::vector<SceneObject> scenes;
public:
  OctreeNode(std::shared_ptr<SceneObject> scene);
};

OctreeNode::OctreeNode(std::shared_ptr<SceneObject> scene) {
  
}

