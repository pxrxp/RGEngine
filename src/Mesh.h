#pragma once
#include "components/BoundingBox.h"
#include <iostream>

class Mesh
{
private:
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_ebo = 0;
  std::vector<glm::vec3> m_vertices_pos;
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

  Mesh(GLuint vao,
       GLuint vbo,
       GLuint ebo,
       std::vector<glm::vec3>& vertices_pos,
       size_t index_count,
       GLenum draw_primitive,
       BoundingBox local_AABB)
    : m_vao(vao)
    , m_vbo(vbo)
    , m_ebo(ebo)
    , m_vertices_pos(std::move(vertices_pos))
    , m_index_count(index_count)
    , m_draw_primitive(draw_primitive)
    , m_local_AABB(local_AABB)
  {
  }

  void draw(unsigned int instance_count = 1);
  const BoundingBox& get_local_AABB() const;

  ~Mesh();
};

#undef SETUP_ATTRIB
