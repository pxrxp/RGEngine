#pragma once
#include "components/BoundingBox.h"
#include "components/vertex_formats.h"
#include "util/obj_model.h"
#include "util/opengl.h"
#include <iostream>
#include <vector>

#define SETUP_ATTRIB(Type, Name)                                               \
  glVertexArrayAttribFormat(m_vao,                                             \
                            attr_index,                                        \
                            sizeof(Type) / sizeof(GLfloat),                    \
                            GL_FLOAT,                                          \
                            GL_FALSE,                                          \
                            offsetof(Vertex, Name));                           \
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
  GLuint get_vbo() const;
  GLuint get_ebo() const;
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
    m_index_count = indices.size() * sizeof(Indices) / sizeof(GLuint);

    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);

    glNamedBufferData(
      m_vbo, m_vertex_count * sizeof(Vertex), vertices.data(), usage);

    if (!indices.empty()) {
      glCreateBuffers(1, &m_ebo);
      glNamedBufferData(
        m_ebo, m_index_count * sizeof(GLuint), indices.data(), usage);
      glVertexArrayElementBuffer(m_vao, m_ebo);
    }

    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));

    int attr_index = 0;
    if constexpr (std::is_same_v<Vertex, Vertex_Pos>) {
      VERTEX_P_FIELDS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosCol>) {
      VERTEX_PC_FIELDS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosTex>) {
      VERTEX_PT_FIELDS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosColNorm>) {
      VERTEX_PCN_FIELDS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosTexNorm>) {
      VERTEX_PTN_FIELDS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, Vertex_PosTexNormTanBitan>) {
      VERTEX_PTNTB_FIELDS(SETUP_ATTRIB)
    }

    for (const auto& vertex : vertices) {
      m_local_AABB.add_point(vertex.position);
    }
  }

  template<typename Vertex, typename Indices>
  void load(const std::string& obj_filename,
            GLenum draw_primitive = GL_TRIANGLES,
            GLenum usage = GL_STATIC_DRAW)
  {
    auto pair = from_OBJ<Vertex, Indices>(obj_filename);
    load<Vertex, Indices>(pair.first, pair.second, draw_primitive, usage);
  }

  void draw(unsigned int instance_count = 1);
  const BoundingBox& get_local_AABB() const;
};

#undef SETUP_ATTRIB
