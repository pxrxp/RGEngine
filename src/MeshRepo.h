#pragma once

#include "Repo.h"
#include "util/opengl.h"
#include <memory>
#include <vector>

class Mesh;
using MeshStrongPtr = std::shared_ptr<Mesh>;
using MeshWeakPtr = std::weak_ptr<Mesh>;

template<typename Vertex, typename Indices>
struct MeshDescriptor
{
  std::string mesh_name;
  std::vector<Vertex> vertices = {};
  std::vector<Indices> indices = {};
  GLenum draw_primitive = GL_TRIANGLES;
  GLenum usage = GL_STATIC_DRAW;
};

class MeshRepo : public RepoBase<std::string, Mesh>
{
public:
  template<typename Vertex, typename Indices>
  MeshStrongPtr load_mesh(const MeshDescriptor<Vertex, Indices>& mesh_desc);
};
