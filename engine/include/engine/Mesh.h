#pragma once
#include <engine/components/BoundingBox.h>
#include <engine/util/definitions.h>
#include <array>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using CollisionTriangle = std::array<glm::vec3, 3>;

class Mesh
{
private:
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_ebo = 0;
  std::vector<glm::vec3> m_vertices_pos;
  size_t m_index_count = 0;
  GLenum m_draw_primitive;
  std::vector<CollisionTriangle> m_collision_triangles;
  BoundingBox m_local_AABB;

public:
#ifndef NDEBUG
  friend std::ostream& operator<<(std::ostream& os, const Mesh& mesh);
#endif


  Mesh(GLuint vao,
       GLuint vbo,
       GLuint ebo,
       std::vector<glm::vec3>& vertices_pos,
       std::vector<CollisionTriangle> collision_triangles,
       size_t index_count,
       GLenum draw_primitive,
       BoundingBox local_AABB)
    : m_vao(vao)
    , m_vbo(vbo)
    , m_ebo(ebo)
    , m_vertices_pos(std::move(vertices_pos))
    , m_index_count(index_count)
    , m_draw_primitive(draw_primitive)
    , m_collision_triangles(std::move(collision_triangles))
    , m_local_AABB(local_AABB)
  {
  }

  void draw(unsigned int instance_count = 1);
  const BoundingBox& get_local_AABB() const;
  const std::vector<CollisionTriangle>& get_collision_triangles() const;
  ~Mesh();

private:
  void bind();
  void unbind();
};

#undef SETUP_ATTRIB


