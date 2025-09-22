#include "Scene.h"
#include <array>
#include <unordered_set>
#include <glm/glm.hpp>
#include <memory>

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
