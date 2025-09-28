#pragma once

#include "Repo.h"
#include "util/opengl.h"
#include <memory>
#include <vector>

class Mesh;
using MeshStrongPtr = std::shared_ptr<Mesh>;
using MeshWeakPtr = std::weak_ptr<Mesh>;

class MeshRepo : public RepoBase<std::string, Mesh>
{
private:
  template<typename Vertex, typename Indices>
  MeshStrongPtr load_mesh(const std::string& obj_filename,
                          GLenum draw_primitive = GL_TRIANGLES,
                          GLenum usage = GL_STATIC_DRAW);

public:
  template<typename Vertex, typename Indices>
  MeshStrongPtr load_mesh(const std::string& mesh_name,
                          const std::vector<Vertex>& vertices,
                          const std::vector<Indices>& indices = {},
                          GLenum draw_primitive = GL_TRIANGLES,
                          GLenum usage = GL_STATIC_DRAW);

  MeshStrongPtr get_mesh(const std::string& path);
};
