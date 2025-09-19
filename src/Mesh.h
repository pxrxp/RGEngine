#pragma once
#include "opengl.h"
#include <glm/glm.hpp>
#include <vector>

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
  glEnableVertexAttribArray(attr_index);                                       \
  glVertexAttribPointer(attr_index++, sizeof(type) / sizeof(GLfloat),          \
                        GL_FLOAT, GL_FALSE, sizeof(Vertex),                    \
                        reinterpret_cast<void *>(offsetof(Vertex, name)));

#define DECLARE_MEMBER(type, name) type name;
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

struct Mesh {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ebo = 0;
  size_t vertex_count = 0;
  size_t index_count = 0;
  GLenum draw_primitive;

  template <typename Vertex>
  void setup(const std::vector<Vertex> &vertices,
             const std::vector<Face> &indices = {},
             GLenum draw_primitive = GL_TRIANGLES,
             GLenum usage = GL_STATIC_DRAW) {

    static_assert(std::is_same<Vertex, VertexColored>::value ||
                  std::is_same<Vertex, VertexTextured>::value ||
                  std::is_same<Vertex, VertexAdvanced>::value);

    this->draw_primitive = draw_primitive;
    this->vertex_count = vertices.size();
    this->index_count = indices.size() * 3;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, this->vertex_count * sizeof(Vertex),
                 vertices.data(), usage);

    if (!indices.empty()) {
      glGenBuffers(1, &ebo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->index_count * sizeof(Face),
                   indices.data(), usage);
    }

    int attr_index = 0;

    if constexpr (std::is_same_v<Vertex, VertexColored>) {
      VERTEX_COLORED_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, VertexTextured>) {
      VERTEX_TEXTURED_MEMBERS(SETUP_ATTRIB)
    } else if constexpr (std::is_same_v<Vertex, VertexAdvanced>) {
      VERTEX_ADVANCED_MEMBERS(SETUP_ATTRIB)
    }

    glBindVertexArray(0);
  }

  void draw() const;
  ~Mesh();
};

#undef SETUP_ATTRIB
#undef VERTEX_COLORED_MEMBERS
#undef VERTEX_TEXTURED_MEMBERS
#undef VERTEX_ADVANCED_MEMBERS
