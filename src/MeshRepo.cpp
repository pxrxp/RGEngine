#include "MeshRepo.h"
#include "components/BoundingBox.h"
#include "components/vertex_formats.h"

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
MeshRepo::load_mesh(const MeshDescriptor<Vertex, Indices>& desc)
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

  if (auto mesh = RepoBase::get(desc.mesh_name); mesh.has_value()) {
    std::cerr << "Mesh " << desc.mesh_name << " already exists!" << std::endl;
    return mesh.value();
  }

  GLuint vao = 0, vbo = 0, ebo = 0;
  GLuint vertex_count = desc.vertices.size();
  GLuint index_count = desc.indices.size() * sizeof(Indices) / sizeof(GLuint);

  glCreateVertexArrays(1, &vao);
  glCreateBuffers(1, &vbo);

  glNamedBufferData(
    vbo, vertex_count * sizeof(Vertex), desc.vertices.data(), desc.usage);

  if (!desc.indices.empty()) {
    glCreateBuffers(1, &ebo);
    glNamedBufferData(
      ebo, index_count * sizeof(GLuint), desc.indices.data(), desc.usage);
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
  for (const auto& vertex : desc.vertices) {
    local_AABB.add_point(vertex.position);
  }

  return RepoBase::create(desc.mesh_name,
                          vao,
                          vbo,
                          ebo,
                          vertex_count,
                          index_count,
                          desc.draw_primitive,
                          local_AABB);
}

#undef SETUP_ATTRIB
