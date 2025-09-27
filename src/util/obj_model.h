#include "../components/vertex_formats.h"
#include "filesystem.h"

#include <charconv>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <optional>
#include <ranges>
#include <string_view>
#include <system_error>
#include <unordered_map>

inline std::optional<float>
try_parse_float(std::string_view sv)
{
  float result;
  auto res = std::from_chars(sv.data(), sv.data() + sv.size(), result);
  if (res.ec == std::errc())
    return result;
  return std::nullopt;
}

inline int
parse_idx(std::string_view sv, size_t storage_len)
{
  int result;
  auto res = std::from_chars(sv.data(), sv.data() + sv.size(), result);
  if (res.ec == std::errc())
    return result < 0 ? result + storage_len : result;
  return -1;
}

// void
// printVerticesMap(const std::unordered_map<std::string_view, size_t>& map)
// {
//   std::cout << "Vertices map:\n";
//   for (const auto& [key, value] : map) {
//     std::cout << key << " -> " << value << "\n";
//   }
//   std::cout << "\n";
// }

// void
// printFaces(const std::vector<std::vector<size_t>>& faces)
// {
//   std::cout << "Faces:\n";
//   for (const auto& f : faces) {
//     std::cout << "[ ";
//     for (auto idx : f)
//       std::cout << idx << " ";
//     std::cout << "]\n";
//   }
//   std::cout << "\n";
// }

// void
// printPositions(const std::vector<Vert>& positions)
// {
//   std::cout << "Positions:\n";
//   for (const auto& v : positions) {
//     std::cout << "(" << v.position[0] << ", " << v.position[1] << ", "
//               << v.position[2] << ")\n";
//   }
//   std::cout << "\n";
// }

// void
// printTex(const std::vector<Vert2>& tex)
// {
//   std::cout << "Texture coordinates:\n";
//   for (const auto& t : tex) {
//     std::cout << "(" << t.position[0] << ", " << t.position[1] << ")\n";
//   }
//   std::cout << "\n";
// }

// void
// printNormals(const std::vector<Vert>& normals)
// {
//   std::cout << "Normals:\n";
//   for (const auto& n : normals) {
//     std::cout << "(" << n.position[0] << ", " << n.position[1] << ", "
//               << n.position[2] << ")\n";
//   }
//   std::cout << "\n";
// }

template<typename Vertex, typename Indices>
std::pair<std::vector<Vertex>, std::vector<Indices>>
from_OBJ(const std::string& path)
{

  static_assert(std::is_same<Vertex, Vertex_Pos>::value ||
                std::is_same<Vertex, Vertex_PosCol>::value ||
                std::is_same<Vertex, Vertex_PosTex>::value ||
                std::is_same<Vertex, Vertex_PosColNorm>::value ||
                std::is_same<Vertex, Vertex_PosTexNorm>::value ||
                std::is_same<Vertex, Vertex_PosTexNormTanBitan>::value);

  std::string obj = Util::read_file("resources/" + path);

  // Each new vertex has a unique id = vertex_count (which increments every
  // time).
  size_t vertex_count = 0;
  std::unordered_map<std::string_view, size_t> vertex_to_idx_map;

  // face_vertices_idx = { face, face, ... }
  // face = { vert_index, vert_index, ... }, ...
  std::vector<std::vector<int>> face_vertices_idx;

  // E.g. For vertex with unique id = 0,
  // position_index = vert_position_idx[0]
  // tex_coords_index = vert_tex_coords_idx[0]
  // normal_index = vert_normals_idx[0]
  //
  // positions = positions_storage[position_index]
  // tex_coords = tex_coords_storage[tex_coord_index]
  // normals = normals_storage[normal_index]
  // ...
  std::vector<int> vert_positions_idx;
  std::vector<int> vert_tex_coords_idx;
  std::vector<int> vert_normals_idx;

  // For temporary use
  // tokens = ["v", "1.2", "3.4", "5.6"]
  // indices = [1,2,3] for f 1/2/3 ...
  std::vector<std::string_view> tokens;
  std::vector<std::string_view> indices;

  std::vector<glm::vec3> positions_storage;
  std::vector<glm::vec2> tex_coords_storage;
  std::vector<glm::vec3> normals_storage;

  for (auto l : obj | std::ranges::views::split('\n')) {
    tokens.clear();

    for (auto t : l | std::ranges::views::split(' ')) {
      if (t.begin() != t.end())
        tokens.emplace_back(&*t.begin(), &*t.end());
    }

    if (tokens.empty())
      continue;

    if (tokens[0] == "v") {
      if (tokens.size() < 4)
        continue;

      if (auto x = try_parse_float(tokens[1]),
          y = try_parse_float(tokens[2]),
          z = try_parse_float(tokens[3]);
          x && y && z) {
        positions_storage.push_back({ *x, *y, *z });
      } else {
        std::cerr << "Invalid entry in OBJ file. Vertex should be specified as "
                     "'v x y z'"
                  << std::endl;
      }
    } else if (tokens[0] == "vt") {
      if (tokens.size() < 3)
        continue;

      if (auto x = try_parse_float(tokens[1]), y = try_parse_float(tokens[2]);
          x && y) {
        tex_coords_storage.push_back({ *x, *y });
      } else {
        std::cerr << "Invalid entry in OBJ file. Texture coordinate should be "
                     "specified as "
                     "'vt x y'"
                  << std::endl;
      }
    } else if (tokens[0] == "vn") {
      if (tokens.size() < 3)
        continue;

      if (auto x = try_parse_float(tokens[1]),
          y = try_parse_float(tokens[2]),
          z = try_parse_float(tokens[3]);
          x && y && z) {
        normals_storage.push_back({ *x, *y, *z });
      } else {
        std::cerr << "Invalid entry in OBJ file. Vertex normal should be "
                     "specified as "
                     "'vn x y z'"
                  << std::endl;
      }
    } else if (tokens[0] == "f") {
      if (tokens.size() < 3)
        continue;

      std::vector<int> face;
      for (size_t i = 1; i < tokens.size(); i++) {
        if (!tokens[i].empty() && tokens[i].back() == '\r')
          tokens[i].remove_suffix(1);

        indices.clear();
        for (auto idx : tokens[i] | std::ranges::views::split('/')) {
          indices.emplace_back(&*idx.begin(), &*idx.end());
        }

        vert_positions_idx.push_back(
          parse_idx(indices[0], positions_storage.size()));
        vert_tex_coords_idx.push_back(
          parse_idx(indices[1], tex_coords_storage.size()));
        vert_normals_idx.push_back(
          parse_idx(indices[2], normals_storage.size()));

        size_t& vert_idx = vertex_to_idx_map[tokens[i]];
        if (vert_idx == 0)
          vert_idx = vertex_count++;
        face.push_back(vert_idx);
      }
      face_vertices_idx.push_back(face);
    }
  }

  std::vector<Vertex_Pos> to_return_vertices;
  std::vector<TriangleIndices> to_return_indices;

  for (auto& pos_i : vert_positions_idx) {
    if (pos_i < positions_storage.size())
      to_return_vertices.push_back({ positions_storage[pos_i] });
  }

  for (auto& face : face_vertices_idx) {
    if (face.size() == 4) {
      to_return_indices.push_back(
        { (GLuint)face[0], (GLuint)face[1], (GLuint)face[2] });
      to_return_indices.push_back(
        { (GLuint)face[0], (GLuint)face[2], (GLuint)face[3] });
    } else if (face.size() == 3) {
      to_return_indices.push_back(
        { (GLuint)face[0], (GLuint)face[1], (GLuint)face[2] });
    }
  }

  return std::pair<std::vector<Vertex>, std::vector<Indices>>(
    std::move(to_return_vertices), std::move(to_return_indices));

  // if constexpr (std::is_same_v<Vertex, Vertex_Pos>) {
  //   if (face_vertices_idx.size() >= 3) {
  //     for (auto&)
  //   }

  //   for (auto& face : face_vertices_idx) {

  //   }
  // } else if constexpr (std::is_same_v<Vertex, Vertex_PosCol>) {

  // } else if constexpr (std::is_same_v<Vertex, Vertex_PosTex>) {
  // } else if constexpr (std::is_same_v<Vertex, Vertex_PosColNorm>) {
  // } else if constexpr (std::is_same_v<Vertex, Vertex_PosTexNorm>) {
  // } else if constexpr (std::is_same_v<Vertex, Vertex_PosTexNormTanBitan>) {
  // }

  // if (tex_coords.empty() && normals.empty()) {
  //   std::vector<SimpleVertex> vertices;
  //   vertices.reserve(positions.size());

  //   std::ranges::views::transform(
  //     positions, vertices.begin(), [](const glm::vec3& position) {
  //       return SimpleVertex{ position };
  //     });

  //   return vertices;
  // }

  // printFaces(faces);
  // printPositions(positions);
  // std::cout << "\n";
  // printTex(tex);
  // std::cout << "\n";
  // printNormals(normals);
  // std::cout << "\n";
  // printVerticesMap(vertices);
}

// const std::vector<Vertex>& vertices,
// const std::vector<Indices>& indices = {},
// GLenum draw_primitive = GL_TRIANGLES,
// GLenum usage = GL_STATIC_DRAW)
