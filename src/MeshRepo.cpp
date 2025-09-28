#include "Repo.h"

MeshRepo::~MeshRepo()
{
  for (auto& mesh : m_resources) {
    if (auto ebo = mesh.second->get_ebo(); ebo)
      glDeleteBuffers(1, &ebo);
    if (auto vbo = mesh.second->get_vbo(); vbo)
      glDeleteBuffers(1, &vbo);
    if (auto vao = mesh.second->get_id(); vao)
      glDeleteVertexArrays(1, &vao);
  }
  m_resources.clear();
}
