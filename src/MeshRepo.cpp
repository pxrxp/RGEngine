#include "MeshRepo.h"
#include "components/BoundingBox.h"
#include "components/vertex_formats.h"
#include "util/obj_model.h"

#define SETUP_ATTRIB(Type, Name)                                               \
  glVertexArrayAttribFormat(vao,                                               \
                            attr_index,                                        \
                            sizeof(Type) / sizeof(GLfloat),                    \
                            GL_FLOAT,                                          \
                            GL_FALSE,                                          \
                            offsetof(Vertex, Name));                           \
  glVertexArrayAttribBinding(vao, attr_index, 0);                              \
  glEnableVertexArrayAttrib(vao, attr_index++);

template<typename Vertex, typename Indices>
MeshStrongPtr
MeshRepo::load_mesh(const std::string& mesh_name,
                    const std::vector<Vertex>& vertices,
                    const std::vector<Indices>& indices,
                    GLenum draw_primitive,
                    GLenum usage)
{

  static_assert(std::is_same<Vertex, Vertex_Pos>::value ||
                std::is_same<Vertex, Vertex_PosCol>::value ||
                std::is_same<Vertex, Vertex_PosTex>::value ||
                std::is_same<Vertex, Vertex_PosColNorm>::value ||
                std::is_same<Vertex, Vertex_PosTexNorm>::value ||
                std::is_same<Vertex, Vertex_PosTexNormTanBitan>::value);

  static_assert(std::is_same<Indices, PointIndex>::value ||
                std::is_same<Indices, LineIndices>::value ||
                std::is_same<Indices, TriangleIndices>::value);

  if (RepoBase::get(mesh_name).has_value()) {
    std::cerr << "Mesh " << mesh_name << " already exists!" << std::endl;
  }

  GLuint vao = 0, vbo = 0, ebo = 0;
  GLuint vertex_count = vertices.size();
  GLuint index_count = indices.size() * sizeof(Indices) / sizeof(GLuint);

  glCreateVertexArrays(1, &vao);
  glCreateBuffers(1, &vbo);

  glNamedBufferData(vbo, vertex_count * sizeof(Vertex), vertices.data(), usage);

  if (!indices.empty()) {
    glCreateBuffers(1, &ebo);
    glNamedBufferData(ebo, index_count * sizeof(GLuint), indices.data(), usage);
    glVertexArrayElementBuffer(vao, ebo);
  }

  glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

  int attr_index = 0;
  if constexpr (std::is_same_v<Vertex, Vertex_Pos>) {
    VERTEX_P_FIELDS(SETUP_ATTRIB)
  } else if constexpr (std::is_same_v<Vertex, Vertex_PosCol>) {
    VERTEX_PC_FIELDS(SETUP_ATTRIB)
  } else if constexpr (std::is_same_v<Vertex, Vertex_PosTex>) {
    VERTEX_PT_FIELDS(SETUP_ATTRIB)
  } else if constexpr (std::is_same_v<Vertex, Vertex_PosColNorm>) {
    VERTEX_PCN_FIELDS(SETUP_ATTRIB)
  } else if constexpr (std::is_same_v<Vertex, Vertex_PosTexNorm>) {
    VERTEX_PTN_FIELDS(SETUP_ATTRIB)
  } else if constexpr (std::is_same_v<Vertex, Vertex_PosTexNormTanBitan>) {
    VERTEX_PTNTB_FIELDS(SETUP_ATTRIB)
  }

  BoundingBox local_AABB;
  for (const auto& vertex : vertices) {
    local_AABB.add_point(vertex.position);
  }

  return RepoBase::create(mesh_name,
                          vao,
                          vbo,
                          ebo,
                          vertex_count,
                          index_count,
                          draw_primitive,
                          local_AABB);
}

template<typename Vertex, typename Indices>
MeshStrongPtr
MeshRepo::load_mesh(const std::string& obj_filename,
                    GLenum draw_primitive,
                    GLenum usage)
{
  auto pair = from_OBJ<Vertex, Indices>(obj_filename);
  return load_mesh<Vertex, Indices>(
    pair.first, pair.second, draw_primitive, usage);
}

#undef SETUP_ATTRIB
