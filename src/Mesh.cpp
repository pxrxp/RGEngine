#include "Mesh.h"
#include "opengl.h"

void Mesh::draw() const {
  glBindVertexArray(vao);
  if (index_count > 0) {
    glDrawElements(this->draw_primitive, static_cast<int>(index_count),
                   GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(this->draw_primitive, 0, static_cast<int>(vertex_count));
  }
  glBindVertexArray(0);
}

Mesh::~Mesh() {
  if (ebo)
    glDeleteBuffers(1, &ebo);
  if (vbo)
    glDeleteBuffers(1, &vbo);
  if (vao)
    glDeleteVertexArrays(1, &vao);
}
