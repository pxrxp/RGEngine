#include "Mesh.h"
#include "opengl.h"

GLuint Mesh::get_id() const { return m_vao; }

void Mesh::bind() {
 glBindVertexArray(m_vao);
}

void Mesh::unbind() {
  glBindVertexArray(0);
}

void Mesh::draw(unsigned int instance_count) {
  this->bind();
  if (instance_count == 0)
    return;

  if (m_index_count > 0) {
    if (instance_count == 1) {
      glDrawElements(m_draw_primitive, static_cast<int>(m_index_count),
                     GL_UNSIGNED_INT, 0);
    } else {
      glDrawElementsInstanced(m_draw_primitive,
                              static_cast<int>(m_index_count), GL_UNSIGNED_INT, 0,
                              instance_count);
    }
  } else {
    if (instance_count == 1) {
      glDrawArrays(m_draw_primitive, 0, static_cast<int>(m_vertex_count));
    } else {
      glDrawArraysInstanced(m_draw_primitive, 0,
                            static_cast<int>(m_vertex_count), instance_count);
    }
  }
  this->unbind();
}

void Mesh::rotate(const glm::vec3 &axis, float angle) {
  m_orientation =
      glm::normalize(glm::angleAxis(angle, axis) * m_orientation);
}
void Mesh::translate(const glm::vec3 &offset) { m_position += offset; }
void Mesh::scale(const glm::vec3 &factors) { m_scale_factors *= factors; }
glm::mat4 Mesh::get_model_matrix() const {
  glm::mat4 T = glm::translate(glm::mat4(1.f), m_position);
  glm::mat4 R = glm::mat4_cast(m_orientation);
  glm::mat4 S = glm::scale(glm::mat4(1.f), m_scale_factors);
  return T * R * S;
}

Mesh::~Mesh() {
 if (m_ebo)
    glDeleteBuffers(1, &m_ebo);
  if (m_vbo)
    glDeleteBuffers(1, &m_vbo);
  if (m_vao)
    glDeleteVertexArrays(1, &m_vao);
}
