#pragma once

#include "../components/BoundingBox.h"
#include "../components/vertex_formats.h"
#include "../util/opengl.h"
#include "RepoBase.h"
#include <memory>
#include <ranges>
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

  bool is_getter_desc() const
  {
    return vertices.empty() && indices.empty() &&
           draw_primitive == GL_TRIANGLES && usage == GL_STATIC_DRAW;
  }

  MeshDescriptor& recalculate_normals(bool ccw = true)
  {
    if constexpr (!requires { Vertex::normal; })
      return *this;

    auto recalc = [](Vertex& v1, Vertex& v2, Vertex& v3) {
      glm::vec3 A = v2.position - v1.position;
      glm::vec3 B = v3.position - v1.position;
      glm::vec3 normal = glm::normalize(glm::cross(A, B));

      v1.normal = glm::normalize(v1.normal + normal);
      v2.normal = glm::normalize(v2.normal + normal);
      v3.normal = glm::normalize(v3.normal + normal);
    };

    if (draw_primitive == GL_TRIANGLES) {
      if (!indices.empty()) {
        if constexpr (std::is_same_v<Indices, TriangleIndices>) {
          for (auto const& index : indices) {
            if (index.vert1 >= vertices.size() ||
                index.vert2 >= vertices.size() ||
                index.vert3 >= vertices.size())
              continue;

            recalc(vertices[index.vert1],
                   vertices[index.vert2],
                   vertices[index.vert2]);
          }
        }
      } else {
        for (size_t i = 0; i <= vertices.size() - 3; i += 3) {
          recalc(vertices[i], vertices[i + 1], vertices[i + 2]);
        }
      }
    } else if (draw_primitive == GL_TRIANGLE_STRIP) {
      if (!indices.empty()) {
        if constexpr (std::is_same_v<Indices, PointIndex>) {
          for (size_t i = 0; i <= indices.size() - 3; i++) {
            auto& idx1 = indices[i];
            auto& idx2 = indices[i + 1];
            auto& idx3 = indices[i + 2];

            if (idx1 >= vertices.size() || idx2 >= vertices.size() ||
                idx3 >= vertices.size())
              continue;

            recalc(vertices[idx1], vertices[idx2], vertices[idx3]);
          }
        }
      } else {
        for (size_t i = 0; i <= vertices.size() - 3; i++) {
          recalc(vertices[i], vertices[i + 1], vertices[i + 2]);
        }
      }
    } else if (draw_primitive == GL_TRIANGLE_FAN) {
      if (!indices.empty()) {
        if constexpr (std::is_same_v<Indices, PointIndex>) {
          auto& idx1 = indices[0];
          if (idx1 < vertices.size()) {
            for (size_t i = 1; i <= indices.size() - 2; i++) {
              auto& idx2 = indices[i];
              auto& idx3 = indices[i + 1];

              if (idx2 >= vertices.size() || idx3 >= vertices.size())
                continue;

              recalc(vertices[idx1], vertices[idx2], vertices[idx3]);
            }
          }
        }
      } else {
        for (size_t i = 1; i <= vertices.size() - 2; i += 2) {
          recalc(vertices[0], vertices[i], vertices[i + 1]);
        }
      }
    }

    if (!ccw) {
      for (auto& vert : vertices) {
        vert.normal = -vert.normal;
      }
    }

    return *this;
  }
};

#define SETUP_ATTRIB(Type, Name)                                               \
  glVertexArrayAttribFormat(vao,                                               \
                            attr_index,                                        \
                            sizeof(Type) / sizeof(GLfloat),                    \
                            GL_FLOAT,                                          \
                            GL_FALSE,                                          \
                            offsetof(Vertex, Name));                           \
  glVertexArrayAttribBinding(vao, attr_index, 0);                              \
  glEnableVertexArrayAttrib(vao, attr_index++);

class MeshRepo : public RepoBase<std::string, Mesh>
{
public:
  template<typename Vertex, typename Indices>
  MeshStrongPtr load_mesh(const MeshDescriptor<Vertex, Indices>& desc)
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
      if (!desc.is_getter_desc())
        std::cerr << "Mesh " << desc.mesh_name << " already exists!"
                  << std::endl;
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
};

#undef SETUP_ATTRIB
