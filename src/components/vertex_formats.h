#pragma once
#include "../util/definitions.h"
#include <glm/glm.hpp>
#include <iostream>

#define VERTEX_P_FIELDS(X) X(glm::vec3, position)

#define VERTEX_PC_FIELDS(X)                                                    \
  X(glm::vec3, position)                                                       \
  X(glm::vec3, color)

#define VERTEX_PT_FIELDS(X)                                                    \
  X(glm::vec3, position)                                                       \
  X(glm::vec2, texture)

#define VERTEX_PCN_FIELDS(X)                                                   \
  X(glm::vec3, position)                                                       \
  X(glm::vec3, color)                                                          \
  X(glm::vec3, normal)

#define VERTEX_PTN_FIELDS(X)                                                   \
  X(glm::vec3, position)                                                       \
  X(glm::vec2, texture)                                                        \
  X(glm::vec3, normal)

#define VERTEX_PTNTB_FIELDS(X)                                                 \
  X(glm::vec3, position)                                                       \
  X(glm::vec2, texture)                                                        \
  X(glm::vec3, normal)                                                         \
  X(glm::vec3, tangent)                                                        \
  X(glm::vec3, bitangent)

#define DECLARE_MEMBER(Type, Name)           Type Name;
#define DECLARE_PARAM_WITH_COMMA(Type, Name) const Type &Name,
#define DECLARE_INIT_WITH_COMMA(Type, Name)  Name(Name),

struct VertexAttribute
{};

#define DEFINE_VERTEX_STRUCT(StructName, Fields)                               \
  struct StructName : VertexAttribute                                          \
  {                                                                            \
  public:                                                                      \
    Fields(DECLARE_MEMBER)                                                     \
                                                                               \
      StructName() = default;                                                  \
                                                                               \
    StructName(Fields(DECLARE_PARAM_WITH_COMMA) int /*unused*/ = 0)            \
      : Fields(DECLARE_INIT_WITH_COMMA) VertexAttribute()                      \
    {                                                                          \
    }                                                                          \
                                                                               \
    template<typename T = StructName>                                          \
    StructName(std::initializer_list<GLfloat> list)                            \
      requires(std::is_same_v<T, Vertex_Pos>)                                  \
    {                                                                          \
      if (list.size() != 3)                                                    \
        throw std::runtime_error("Position requires 3 elements");              \
      auto it = list.begin();                                                  \
      this->position.x = *it++;                                                \
      this->position.y = *it++;                                                \
      this->position.z = *it++;                                                \
    }                                                                          \
  };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"

DEFINE_VERTEX_STRUCT(Vertex_Pos, VERTEX_P_FIELDS)
DEFINE_VERTEX_STRUCT(Vertex_PosCol, VERTEX_PC_FIELDS)
DEFINE_VERTEX_STRUCT(Vertex_PosTex, VERTEX_PT_FIELDS)
DEFINE_VERTEX_STRUCT(Vertex_PosColNorm, VERTEX_PCN_FIELDS)
DEFINE_VERTEX_STRUCT(Vertex_PosTexNorm, VERTEX_PTN_FIELDS)
DEFINE_VERTEX_STRUCT(Vertex_PosTexNormTanBitan, VERTEX_PTNTB_FIELDS)

#pragma GCC diagnostic pop

struct PointIndex
{
  GLuint vert;
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
operator<<(std::ostream& os, const Vertex_PosCol& v)
{
  os << "Vertex_PosCol{ pos: " << v.position << ", col: " << v.color << " }";
  return os;
}

inline std::ostream&
operator<<(std::ostream& os, const Vertex_PosTex& v)
{
  os << "Vertex_PosTex{ pos: " << v.position << ", tex: " << v.texture << " }";
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
  os << "Vertex_PosTexNormTanBitan{ pos: " << v.position
     << ", nrm: " << v.normal << ", tex: " << v.texture
     << ", tan: " << v.tangent << ", bitan: " << v.bitangent << " }";
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
