#pragma once
#include "../util/definitions.h"
#include <glm/glm.hpp>
#include <iostream>

#define VERTEX_P_MEMBERS(X) X(Vertex_Pos, glm::vec3, position)

#define VERTEX_PC_MEMBERS(X)                                                   \
  X(Vertex_PosCol, glm::vec3, position)                                        \
  X(Vertex_PosCol, glm::vec3, color)

#define VERTEX_PT_MEMBERS(X)                                                   \
  X(Vertex_PosTex, glm::vec3, position)                                        \
  X(Vertex_PosTex, glm::vec2, texture)

#define VERTEX_PCN_MEMBERS(X)                                                  \
  X(Vertex_PosColNorm, glm::vec3, position)                                        \
  X(Vertex_PosColNorm, glm::vec3, normal)                                          \
  X(Vertex_PosColNorm, glm::vec3, color)

#define VERTEX_PTN_MEMBERS(X)                                                  \
  X(Vertex_PosTexNorm, glm::vec3, position)                                       \
  X(Vertex_PosTexNorm, glm::vec3, normal)                                         \
  X(Vertex_PosTexNorm, glm::vec2, texture)

#define VERTEX_PTNTB_MEMBERS(X)                                                \
  X(Vertex_PosTexNormTanBitan, glm::vec3, position)                                       \
  X(Vertex_PosTexNormTanBitan, glm::vec3, normal)                                         \
  X(Vertex_PosTexNormTanBitan, glm::vec2, texture)                                        \
  X(Vertex_PosTexNormTanBitan, glm::vec2, tangent)                                        \
  X(Vertex_PosTexNormTanBitan, glm::vec2, bitangent)

#define DECLARE_MEMBER(class, type, name) type name;

struct Vertex_Pos
{
  VERTEX_P_MEMBERS(DECLARE_MEMBER)

  Vertex_Pos(const glm::vec3& position_)
    : position(position_)
  {
  }
  Vertex_Pos(std::initializer_list<GLfloat> position_)
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

struct Vertex_PosCol
{
  VERTEX_PCN_MEMBERS(DECLARE_MEMBER)
};

struct Vertex_PosTex
{
  VERTEX_PTN_MEMBERS(DECLARE_MEMBER)
};

struct Vertex_PosColNorm
{
  VERTEX_PCN_MEMBERS(DECLARE_MEMBER)
};

struct Vertex_PosTexNorm
{
  VERTEX_PTN_MEMBERS(DECLARE_MEMBER)
};

struct Vertex_PosTexNormTanBitan
{
  VERTEX_PTNTB_MEMBERS(DECLARE_MEMBER)
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

inline std::ostream&
operator<<(std::ostream& os, const glm::vec2& vec)
{
  os << "(" << vec.x << ", " << vec.y << ")";
  return os;
}

inline std::ostream&
operator<<(std::ostream& os, const glm::vec3& vec)
{
  os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
  return os;
}

inline std::ostream&
operator<<(std::ostream& os, const Vertex_Pos& v)
{
  os << "Vertex_Pos{ pos: " << v.position << " }";
  return os;
}

inline std::ostream&
operator<<(std::ostream& os, const Vertex_PosColNorm& v)
{
  os << "Vertex_PosColNorm{ pos: " << v.position << ", nrm: " << v.normal
     << ", col: " << v.color << " }";
  return os;
}

inline std::ostream&
operator<<(std::ostream& os, const Vertex_PosTexNorm& v)
{
  os << "Vertex_PosTexNorm{ pos: " << v.position << ", nrm: " << v.normal
     << ", tex: " << v.texture << " }";
  return os;
}

inline std::ostream&
operator<<(std::ostream& os, const Vertex_PosTexNormTanBitan& v)
{
  os << "Vertex_PosTexNormTanBitan{ pos: " << v.position << ", nrm: " << v.normal
     << ", tex: " << v.texture << ", tan: " << v.tangent
     << ", bitan: " << v.bitangent << " }";
  return os;
}

inline std::ostream&
operator<<(std::ostream& os, const PointIndex& i)
{
  os << "Point(" << i.vert << ")";
  return os;
}

inline std::ostream&
operator<<(std::ostream& os, const LineIndices& i)
{
  os << "Line(" << i.vert1 << ", " << i.vert2 << ")";
  return os;
}

inline std::ostream&
operator<<(std::ostream& os, const TriangleIndices& i)
{
  os << "Triangle(" << i.vert1 << ", " << i.vert2 << ", " << i.vert3 << ")";
  return os;
}
#endif
