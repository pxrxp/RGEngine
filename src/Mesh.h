#pragma once
#include "components/BoundingBox.h"
#include "components/vertex_formats.h"
#include "util/opengl.h"
#include <iostream>
#include <vector>

#define SETUP_ATTRIB(class, type, name)                                        \
  glVertexArrayAttribFormat(m_vao,                                             \
                            attr_index,                                        \
                            sizeof(type) / sizeof(GLfloat),                    \
                            GL_FLOAT,                                          \
                            GL_FALSE,                                          \
                            offsetof(Vertex, name));                           \
  glVertexArrayAttribBinding(m_vao, attr_index, 0);                            \
  glEnableVertexArrayAttrib(m_vao, attr_index++);

class Mesh
{
private:
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_ebo = 0;
  size_t m_vertex_count = 0;
  size_t m_index_count = 0;
  GLenum m_draw_primitive;

  BoundingBox m_local_AABB;

public:
#ifndef NDEBUG
  friend std::ostream& operator<<(std::ostream& os, const Mesh& mesh);
#endif

  GLuint get_id() const;
  void bind();
  void unbind();

  template<typename Vertex, typename Indices>
  void load(const std::vector<Vertex>& vertices,
            const std::vector<Indices>& indices = {},
            GLenum draw_primitive = GL_TRIANGLES,
            GLenum usage = GL_STATIC_DRAW)
  {

    static_assert(std::is_same<Vertex, Vertex_Pos>::value ||
                  std::is_same<Vertex, Vertex_PosCol>::value ||
                  std::is_same<Vertex, Vertex_PosTex>::value ||
                  std::is_same<Vertex, Vertex_PosColNorm>::value ||
                  std::is_same<Vertex, Vertex_PosTexNorm>::value ||
                  std::is_same<Vertex, Vertex_PosTexNormTanBitan>::value);

    static_assert(std::is_same<Indices, PointIndex>::value ||
                  std::is_same<Indices, LineIndices>::value ||
                  std::is_same<Indices, TriangleIndices>::value);

    m_draw_primitive = draw_primitive;
    m_vertex_count = vertices.size();

    if constexpr (std::is_same_v<Indices, TriangleIndices>) {
      m_index_count = indices.size() * 3;
    } else if constexpr (std::is_same_v<Indices, LineIndices>) {
      m_index_count = indices.size() * 2;
    } else if constexpr (std::is_same_v<Indices, PointIndex>) {
      m_index_count = indices.size();
    }

    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);

    glNamedBufferData(
      m_vbo, m_vertex_count * sizeof(Vertex), vertices.data(), usage);

    if (!indices.empty()) {
      glCreateBuffers(1, &m_ebo);
      glNamedBufferData(
        m_ebo, m_index_count * sizeof(Indices), indices.data(), usage);
      glVertexArrayElementBuffer(m_vao, m_ebo);
    }

    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));

    int attr_index = 0;

    if constexpr (std::is_same_v<Vertex, Vertex_Pos>) {
      VERTEX_P_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosCol>) {
      VERTEX_PC_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosTex>) {
      VERTEX_PT_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosColNorm>) {
      VERTEX_PCN_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosTexNorm>) {
      VERTEX_PTN_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosTexNormTanBitan>) {
      VERTEX_PTNTB_MEMBERS(SETUP_ATTRIB)
    }

    for (const auto& vertex : vertices) {
      m_local_AABB.add_point(vertex.position);
    }
  }

  void draw(unsigned int instance_count = 1);
  const BoundingBox& get_local_AABB() const;
  ~Mesh();
};

#undef SETUP_ATTRIB
