#include "components/BoundingBox.h"
#include <glm/glm.hpp>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#ifndef NDEBUG
template<typename T>
class OctreeNode;
template<typename T>
void
print_node_recursive(std::ostream& os,
                     const OctreeNode<T>& node,
                     const std::string& indent,
                     bool is_last)
{
  os << indent << (is_last ? "\\-- " : "+-- ");
  os << "Node [" << node.m_boundary << "]";

  std::visit(
    [&](const auto& children) {
      using ChildrenType = std::decay_t<decltype(children)>;
      if constexpr (std::is_same_v<ChildrenType,
                                   typename OctreeNode<T>::DataList>) {
        os << " (Leaf) | Data Points: " << children.size() << "\n";
        std::string dataIndent = indent + (is_last ? "    " : "|   ");
        for (size_t i = 0; i < children.size(); ++i) {
          os << dataIndent << (i == children.size() - 1 ? "\\-- " : "+-- ");
          os << "Data: " << children[i]->data << " @ " << children[i]->center
             << "\n";
        }
      }
      if constexpr (std::is_same_v<ChildrenType,
                                   typename OctreeNode<T>::NodeList>) {
        os << " (Internal)\n";
        std::string child_indent = indent + (is_last ? "    " : "|   ");
        for (size_t i = 0; i < children.size(); ++i) {
          print_node_recursive(
            os, *children[i], child_indent, i == children.size() - 1);
        }
      }
    },
    node.m_children);
}

template<typename T>
class Octree;

template<typename T>
std::ostream&
operator<<(std::ostream& os, const Octree<T>& octree)
{
  if (octree.m_root) {
    os << "Octree Structure:\n";
    print_node_recursive(os, *octree.m_root, "", true);
  } else {
    os << "Octree is empty.\n";
  }
  return os;
}
#endif

template<typename T>
struct OctreeData
{
  T data;
  glm::vec3 center;

  OctreeData(T data, glm::vec3 center)
    : data(std::move(data))
    , center(std::move(center))
  {
  }
};

template<typename T>
class OctreeNode : public std::enable_shared_from_this<OctreeNode<T>>
{
private:
  using NodePtr = std::shared_ptr<OctreeNode<T>>;
  using DataPtr = std::shared_ptr<OctreeData<T>>;
  using NodeList = std::vector<NodePtr>;
  using DataList = std::vector<DataPtr>;

  std::variant<NodeList, DataList> m_children;
  BoundingBox m_boundary;

  static const int LEAF_CAPACITY = 8;

  void subdivide()
  {
    DataList current_data = std::get<DataList>(std::move(m_children));

    glm::vec3 min = m_boundary.min;
    glm::vec3 max = m_boundary.max;
    glm::vec3 centroid = (min + max) / 2.0f;

    NodeList new_children;
    new_children.reserve(8);
    new_children.push_back(
      std::make_shared<OctreeNode>(BoundingBox{ min, centroid }));
    new_children.push_back(std::make_shared<OctreeNode>(BoundingBox{
      { centroid.x, min.y, min.z }, { max.x, centroid.y, centroid.z } }));
    new_children.push_back(std::make_shared<OctreeNode>(BoundingBox{
      { min.x, centroid.y, min.z }, { centroid.x, max.y, centroid.z } }));
    new_children.push_back(std::make_shared<OctreeNode>(BoundingBox{
      { centroid.x, centroid.y, min.z }, { max.x, max.y, centroid.z } }));
    new_children.push_back(std::make_shared<OctreeNode>(BoundingBox{
      { min.x, min.y, centroid.z }, { centroid.x, centroid.y, max.z } }));
    new_children.push_back(std::make_shared<OctreeNode>(BoundingBox{
      { centroid.x, min.y, centroid.z }, { max.x, centroid.y, max.z } }));
    new_children.push_back(std::make_shared<OctreeNode>(BoundingBox{
      { min.x, centroid.y, centroid.z }, { centroid.x, max.y, max.z } }));
    new_children.push_back(
      std::make_shared<OctreeNode>(BoundingBox{ centroid, max }));

    m_children = std::move(new_children);

    for (auto& data_point : current_data) {
      insert(data_point);
    }
  }

public:
  OctreeNode(const BoundingBox& aabb)
    : m_boundary(aabb)
  {
    m_children.template emplace<DataList>().reserve(LEAF_CAPACITY);
  }

  BoundingBox get_boundary() const { return m_boundary; }

  bool insert(const DataPtr& data)
  {
    if (m_boundary.where(data->center) == Outside) {
      return false;
    }

    bool inserted = false;
    std::visit(
      [&](auto& children) {
        using ChildrenType = std::decay_t<decltype(children)>;

        if constexpr (std::is_same_v<ChildrenType, DataList>) {
          if (children.size() < LEAF_CAPACITY) {
            children.push_back(data);
            inserted = true;
          } else {
            subdivide();
            inserted = insert(data);
          }
        }

        if constexpr (std::is_same_v<ChildrenType, NodeList>) {
          for (auto& child : children) {
            if (child->insert(data)) {
              inserted = true;
              break;
            }
          }
        }
      },
      m_children);

    return inserted;
  }

  void find(const BoundingBox& queryBox, std::vector<NodePtr>& result)
  {
    if (!m_boundary.intersects(queryBox)) {
      return;
    }

    if (std::holds_alternative<NodeList>(m_children)) {
      auto& children = std::get<NodeList>(m_children);
      for (const auto& child : children) {
        child->find(queryBox, result);
      }
    } else {
      result.push_back(this->shared_from_this());
    }
  }

#ifndef NDEBUG
  friend void print_node_recursive<>(std::ostream& os,
                                     const OctreeNode<T>& node,
                                     const std::string& indent,
                                     bool is_last);
#endif
};

template<typename T>
class Octree
{
private:
  std::shared_ptr<OctreeNode<T>> m_root;

public:
  Octree(const BoundingBox& boundary)
  {
    m_root = std::make_shared<OctreeNode<T>>(boundary);
  }

  bool insert(const T& data, const glm::vec3& center)
  {
    auto data_ptr = std::make_shared<OctreeData<T>>(data, center);
    return m_root->insert(data_ptr);
  }

  std::vector<std::shared_ptr<OctreeNode<T>>> find(const BoundingBox& box)
  {
    std::vector<std::shared_ptr<OctreeNode<T>>> result;
    m_root->find(box, result);
    return result;
  }

#ifndef NDEBUG
  friend std::ostream& operator<< <>(std::ostream& os, const Octree<T>& octree);
#endif
};
