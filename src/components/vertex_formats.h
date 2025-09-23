#pragma once
#include "../opengl.h"
#include <array>
#include <glm/glm.hpp>
#include <initializer_list>
#include <stdexcept>
#include <vector>

#define SIMPLE_VERTEX_MEMBERS(X) X(SimpleVertex, glm::vec3, position)

#define COLORED_VERTEX_MEMBERS(X)                                              \
  X(ColoredVertex, glm::vec3, position)                                        \
  X(ColoredVertex, glm::vec3, normal)                                          \
  X(ColoredVertex, glm::vec3, color)

#define TEXTURED_VERTEX_MEMBERS(X)                                             \
  X(TexturedVertex, glm::vec3, position)                                       \
  X(TexturedVertex, glm::vec3, normal)                                         \
  X(TexturedVertex, glm::vec2, texture)

#define ADVANCED_VERTEX_MEMBERS(X)                                             \
  X(AdvancedVertex, glm::vec3, position)                                       \
  X(AdvancedVertex, glm::vec3, normal)                                         \
  X(AdvancedVertex, glm::vec2, texture)                                        \
  X(AdvancedVertex, glm::vec2, tangent)                                        \
  X(AdvancedVertex, glm::vec2, bitangent)

#define DECLARE_MEMBER(class, type, name) type name;

struct SimpleVertex {
  SIMPLE_VERTEX_MEMBERS(DECLARE_MEMBER)

  // SimpleVertex(const glm::vec3 &position_) : position(position_) {}

  // SimpleVertex(std::initializer_list<GLfloat> position_) {
  //   if (position_.size() != 3)
  //     throw std::runtime_error("TriangleIndices requires exactly 3 elements");
  //   auto it = position_.begin();
  //   position.x = *it++;
  //   position.y = *it++;
  //   position.z = *it++;
  // }

  // SimpleVertex(const std::array<GLfloat, 3> &position_) {
  //   position.x = position_[0];
  //   position.y = position_[1];
  //   position.z = position_[2];
  // }

  // SimpleVertex(const std::vector<GLfloat> &position_) {
  //   if (position_.size() != 3)
  //     throw std::runtime_error("TriangleIndices requires exactly 3 elements");
  //   position.x = position_[0];
  //   position.y = position_[1];
  //   position.z = position_[2];
  // }
};

struct ColoredVertex {
  COLORED_VERTEX_MEMBERS(DECLARE_MEMBER)
};

struct TexturedVertex {
  TEXTURED_VERTEX_MEMBERS(DECLARE_MEMBER)
};

struct AdvancedVertex {
  ADVANCED_VERTEX_MEMBERS(DECLARE_MEMBER)
};
#undef DECLARE_MEMBER

struct PointIndex {
  GLuint vert;
  PointIndex(GLuint vert) : vert(vert) {}
};

struct LineIndices {
  GLuint vert1, vert2;
};

struct TriangleIndices {
  GLuint vert1, vert2, vert3;
};
