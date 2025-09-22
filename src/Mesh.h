#pragma once
#include "opengl.h"
#include <vector>

#define VERTEX_SIMPLE_MEMBERS(X) X(glm::vec3, position)

#define VERTEX_COLORED_MEMBERS(X)                                              \
  X(glm::vec3, position)                                                       \
  X(glm::vec3, normal)                                                         \
  X(glm::vec3, color)

#define VERTEX_TEXTURED_MEMBERS(X)                                             \
  X(glm::vec3, position)                                                       \
  X(glm::vec3, normal)                                                         \
  X(glm::vec2, texture)

#define VERTEX_ADVANCED_MEMBERS(X)                                             \
  X(glm::vec3, position)                                                       \
  X(glm::vec3, normal)                                                         \
  X(glm::vec2, texture)                                                        \
  X(glm::vec2, tangent)                                                        \
  X(glm::vec2, bitangent)

#define SETUP_ATTRIB(type, name)                                               \
  glVertexArrayAttribFormat(m_vao, attr_index, sizeof(type) / sizeof(GLfloat), \
                            GL_FLOAT, GL_FALSE, offsetof(Vertex, name));       \
  glVertexArrayAttribBinding(m_vao, attr_index, 0);                            \
  glEnableVertexArrayAttrib(m_vao, attr_index++);

#define DECLARE_MEMBER(type, name) type name;
struct VertexSimple {
  VERTEX_SIMPLE_MEMBERS(DECLARE_MEMBER)
};

struct VertexColored {
  VERTEX_COLORED_MEMBERS(DECLARE_MEMBER)
};

struct VertexTextured {
  VERTEX_TEXTURED_MEMBERS(DECLARE_MEMBER)
};

struct VertexAdvanced {
  VERTEX_ADVANCED_MEMBERS(DECLARE_MEMBER)
};
#undef DECLARE_MEMBER

struct Face {
  GLuint vert1, vert2, vert3;
};

class Mesh {
private:
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_ebo = 0;
  size_t m_vertex_count = 0;
  size_t m_index_count = 0;
  GLenum m_draw_primitive;

public:
  GLuint get_id() const;
  void bind();
  void unbind();

  template <typename Vertex>
  void load(const std::vector<Vertex> &vertices,
             const std::vector<Face> &indices = {},
             GLenum draw_primitive = GL_TRIANGLES,
             GLenum usage = GL_STATIC_DRAW) {

    static_assert(std::is_same<Vertex, VertexSimple>::value ||
                  std::is_same<Vertex, VertexColored>::value ||
                  std::is_same<Vertex, VertexTextured>::value ||
                  std::is_same<Vertex, VertexAdvanced>::value);

    m_draw_primitive = draw_primitive;
    m_vertex_count = vertices.size();
    m_index_count = indices.size() * 3;

    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);

    glNamedBufferData(m_vbo, m_vertex_count * sizeof(Vertex), vertices.data(),
                      usage);

    if (!indices.empty()) {
      glCreateBuffers(1, &m_ebo);
      glNamedBufferData(m_ebo, m_index_count * sizeof(Face), indices.data(),
                        usage);
      glVertexArrayElementBuffer(m_vao, m_ebo);
    }

    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));

    int attr_index = 0;

    if constexpr (std::is_same_v<Vertex, VertexSimple>) {
      VERTEX_SIMPLE_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, VertexColored>) {
      VERTEX_COLORED_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, VertexTextured>) {
      VERTEX_TEXTURED_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, VertexAdvanced>) {
      VERTEX_ADVANCED_MEMBERS(SETUP_ATTRIB)
    }
  }

  void draw(unsigned int instance_count = 1);
  ~Mesh();
};

#undef SETUP_ATTRIB
#undef VERTEX_SIMPLE_MEMBERS
#undef VERTEX_COLORED_MEMBERS
#undef VERTEX_TEXTURED_MEMBERS
#undef VERTEX_ADVANCED_MEMBERS
