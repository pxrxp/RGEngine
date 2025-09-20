#include "Mesh.h"
#include "opengl.h"

GLuint Mesh::get_id() const { return vao; }

void Mesh::bind() { glBindVertexArray(vao); }

void Mesh::unbind() { glBindVertexArray(0); }

void Mesh::draw(bool bind, unsigned int instance_count) const {
  if (instance_count == 0)
    return;

  if (bind)
    glBindVertexArray(vao);

  if (index_count > 0) {
    if (instance_count == 1) {
      glDrawElements(this->draw_primitive, static_cast<int>(index_count),
                     GL_UNSIGNED_INT, 0);
    } else {
      glDrawElementsInstanced(this->draw_primitive,
                              static_cast<int>(index_count), GL_UNSIGNED_INT, 0,
                              instance_count);
    }
  } else {
    if (instance_count == 1) {
      glDrawArrays(this->draw_primitive, 0, static_cast<int>(vertex_count));
    } else {
      glDrawArraysInstanced(this->draw_primitive, 0,
                            static_cast<int>(vertex_count), instance_count);
    }
  }

  if (bind)
    glBindVertexArray(0);
}

void Mesh::rotate(const glm::vec3 &axis, float angle) {
  this->orientation =
      glm::normalize(glm::angleAxis(angle, axis) * this->orientation);
}
void Mesh::translate(const glm::vec3 &offset) { this->position += offset; }
void Mesh::scale(const glm::vec3 &factors) { this->scale_factors *= factors; }
glm::mat4 Mesh::get_model_matrix() const {
  glm::mat4 T = glm::translate(glm::mat4(1.f), position);
  glm::mat4 R = glm::mat4_cast(orientation);
  glm::mat4 S = glm::scale(glm::mat4(1.f), scale_factors);
  return T * R * S;
}

Mesh::~Mesh() {
  if (ebo)
    glDeleteBuffers(1, &ebo);
  if (vbo)
    glDeleteBuffers(1, &vbo);
  if (vao)
    glDeleteVertexArrays(1, &vao);
}
