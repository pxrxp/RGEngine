#include <engine/Mesh.h>
#include <engine/util/log.h>
#include <engine/util/opengl.h>

Mesh::~Mesh()
{
#ifndef NDEBUG
  Util::log_out() << "Destroying Mesh ID: " << m_vao << std::endl;
#endif

  if (m_ebo) {
    glDeleteBuffers(1, &m_ebo);

#ifndef NDEBUG
    Util::log_out() << "Deleted element buffer object " << m_ebo << std::endl;
#endif
  }
  if (m_vbo) {
    glDeleteBuffers(1, &m_vbo);

#ifndef NDEBUG
    Util::log_out() << "Deleted vertex buffer object " << m_vbo << std::endl;
#endif
  }
  if (m_vao) {
    glDeleteVertexArrays(1, &m_vao);

#ifndef NDEBUG
    Util::log_out() << "Deleted vertex array object " << m_vao << std::endl;
#endif
  }

#ifndef NDEBUG
  Util::log_out() << std::endl;
#endif
}

void
Mesh::bind()
{
  glBindVertexArray(m_vao);
}

void
Mesh::unbind()
{
  glBindVertexArray(0);
}

void
Mesh::draw(unsigned int instance_count)
{
  this->bind();
  if (instance_count == 0)
    return;

  if (m_index_count > 0) {
    if (instance_count == 1) {
      glDrawElements(
        m_draw_primitive, static_cast<int>(m_index_count), GL_UNSIGNED_INT, 0);
    } else {
      glDrawElementsInstanced(m_draw_primitive,
                              static_cast<int>(m_index_count),
                              GL_UNSIGNED_INT,
                              0,
                              instance_count);
    }
  } else {
    if (instance_count == 1) {
      glDrawArrays(
        m_draw_primitive, 0, static_cast<int>(m_vertices_pos.size()));
    } else {
      glDrawArraysInstanced(m_draw_primitive,
                            0,
                            static_cast<int>(m_vertices_pos.size()),
                            instance_count);
    }
  }
  this->unbind();
}

const BoundingBox&
Mesh::get_local_AABB() const
{
  return m_local_AABB;
}

const std::vector<CollisionTriangle>&
Mesh::get_collision_triangles() const
{
  return m_collision_triangles;
}

#ifndef NDEBUG
std::ostream&
operator<<(std::ostream& os, const Mesh& mesh)
{
  os << "Mesh(vao=" << mesh.m_vao << ", vbo=" << mesh.m_vbo
     << ", ebo=" << mesh.m_ebo
     << ", vertex_count=" << mesh.m_vertices_pos.size()
     << ", index_count=" << mesh.m_index_count
     << ", draw_primitive=" << mesh.m_draw_primitive
     << ", local_AABB=" << mesh.m_local_AABB << ")";
  return os;
}
#endif
