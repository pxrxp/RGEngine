#pragma once
#include "../util/definitions.h"
#include <glm/glm.hpp>
#include <iostream>

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

struct SimpleVertex
{
  SIMPLE_VERTEX_MEMBERS(DECLARE_MEMBER)

  SimpleVertex(const glm::vec3& position_)
    : position(position_)
  {
  }
  SimpleVertex(std::initializer_list<GLfloat> position_)
  {
    if (position_.size() != 3)
      throw std::runtime_error(
        "Position requires exactly 3 elements (x, y, z)");

    auto it = position_.begin();
    position.x = *it++;
    position.y = *it++;
    position.z = *it++;
  }
};

struct ColoredVertex
{
  COLORED_VERTEX_MEMBERS(DECLARE_MEMBER)
};

struct TexturedVertex
{
  TEXTURED_VERTEX_MEMBERS(DECLARE_MEMBER)
};

struct AdvancedVertex
{
  ADVANCED_VERTEX_MEMBERS(DECLARE_MEMBER)
};
#undef DECLARE_MEMBER

struct PointIndex
{
  GLuint vert;
  PointIndex(GLuint vert)
    : vert(vert)
  {
  }
};

struct LineIndices
{
  GLuint vert1, vert2;
};

struct TriangleIndices
{
  GLuint vert1, vert2, vert3;
};


#ifndef NDEBUG

inline std::ostream& operator<<(std::ostream& os, const glm::vec2& vec) {
    os << "(" << vec.x << ", " << vec.y << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec3& vec) {
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const SimpleVertex& v) {
    os << "SimpleVertex{ pos: " << v.position << " }";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const ColoredVertex& v) {
    os << "ColoredVertex{ pos: " << v.position
       << ", nrm: " << v.normal
       << ", col: " << v.color << " }";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const TexturedVertex& v) {
    os << "TexturedVertex{ pos: " << v.position
       << ", nrm: " << v.normal
       << ", tex: " << v.texture << " }";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const AdvancedVertex& v) {
    os << "AdvancedVertex{ pos: " << v.position
       << ", nrm: " << v.normal
       << ", tex: " << v.texture
       << ", tan: " << v.tangent
       << ", bitan: " << v.bitangent << " }";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const PointIndex& i) {
    os << "Point(" << i.vert << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const LineIndices& i) {
    os << "Line(" << i.vert1 << ", " << i.vert2 << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const TriangleIndices& i) {
    os << "Triangle(" << i.vert1 << ", " << i.vert2 << ", " << i.vert3 << ")";
    return os;
}
#endif
