# Meshes

## Descriptors

```cpp
template<typename Vertex, typename Indices>
struct MeshDescriptor
{
  std::string mesh_name;
  std::vector<Vertex> vertices;
  std::vector<Indices> indices;
  GLenum draw_primitive = GL_TRIANGLES;
  GLenum usage = GL_STATIC_DRAW;
};
```

Vertex types (`engine/include/engine/components/vertex_formats.h`):

- `Vertex_Pos` — position
- `Vertex_PosCol` — position, colour
- `Vertex_PosTex` — position, UV
- `Vertex_PosColNorm` — position, colour, normal
- `Vertex_PosTexNorm` — position, UV, normal
- `Vertex_PosTexNormTanBitan` — position, UV, normal, tangent, bitangent

Index types: `PointIndex`, `LineIndices`, `TriangleIndices`. The vertex/index combination is checked by `static_assert` in `MeshRepo::load_mesh`.

`MeshDescriptor` also offers `recalculate_normals(bool ccw)` (debug-friendly helper that averages face normals) and a `print()` helper in debug builds.

## Loading

`MeshRepo::load_mesh(desc, reload = false)` caches by `mesh_name`. On a miss it creates a VAO and buffers, uploads vertices and (optional) indices, configures attributes from the vertex macro fields, collects vertex positions and an AABB, and extracts collision triangles for `GL_TRIANGLES` meshes with triangle or point indices.

`reload = true` removes the cached mesh first, creating new GL objects. Note that a cached mesh keeps the geometry of its first descriptor; changing a descriptor's data under the same name requires `reload`.

## Drawing

`Mesh::draw(instance_count)` binds the VAO and draws indexed (`glDrawElements` / `glDrawElementsInstanced`) or non-indexed (`glDrawArrays` / `glDrawArraysInstanced`). `draw` unbinds the VAO afterwards.

## Accessors

`get_local_AABB()` returns the mesh-space bounding box used for world AABBs. `get_collision_triangles()` returns the triangle list used when registering the mesh with a `CollisionWorld`.
