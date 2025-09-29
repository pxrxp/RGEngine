#pragma once

#include "../components/vertex_formats.h"
#include "../MeshRepo.h"
#include "filesystem.h"

#include <charconv>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <optional>
#include <ranges>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

inline std::optional<float>
try_parse_float(std::string_view sv)
{
  float result;
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
  if (ec == std::errc())
    return result;
  return std::nullopt;
}

inline std::optional<int>
parse_and_validate_idx(std::string_view sv, size_t storage_len)
{
  int result;
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);

  if (ec != std::errc() || storage_len == 0) {
    return std::nullopt;
  }

  int final_index =
    (result > 0) ? result - 1 : static_cast<int>(storage_len) + result;

  if (final_index >= 0 && static_cast<size_t>(final_index) < storage_len) {
    return final_index;
  }

  return std::nullopt;
}

struct VertexKey
{
  int pos_idx = -1;
  int tex_idx = -1;
  int norm_idx = -1;

  bool operator==(const VertexKey& other) const
  {
    return pos_idx == other.pos_idx && tex_idx == other.tex_idx &&
           norm_idx == other.norm_idx;
  }
};

struct VertexKeyHasher
{
  std::size_t operator()(const VertexKey& k) const
  {
    size_t h1 = std::hash<int>{}(k.pos_idx);
    size_t h2 = std::hash<int>{}(k.tex_idx);
    size_t h3 = std::hash<int>{}(k.norm_idx);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};

template<typename Vertex, typename Indices>
MeshDescriptor<Vertex, Indices>
from_OBJ(const std::string& path, GLenum draw_primitive, GLenum usage)
{
  static_assert(std::is_same<Vertex, Vertex_Pos>::value ||
                std::is_same<Vertex, Vertex_PosCol>::value ||
                std::is_same<Vertex, Vertex_PosTex>::value ||
                std::is_same<Vertex, Vertex_PosColNorm>::value ||
                std::is_same<Vertex, Vertex_PosTexNorm>::value ||
                std::is_same<Vertex, Vertex_PosTexNormTanBitan>::value);

  const std::string obj_content = Util::read_file("resources/" + path);

  std::vector<Vertex> final_vertices;
  std::vector<Indices> final_indices;
  std::unordered_map<VertexKey, GLuint, VertexKeyHasher> unique_vertices;

  std::vector<glm::vec3> positions_storage;
  std::vector<glm::vec2> tex_coords_storage;
  std::vector<glm::vec3> normals_storage;

  std::vector<std::string_view> tokens;
  std::vector<std::string_view> face_vertex_tokens;

  for (const auto line_view : obj_content | std::ranges::views::split('\n')) {
    tokens.clear();
    for (const auto token_view : line_view | std::ranges::views::split(' ')) {
      if (!token_view.empty()) {
        tokens.emplace_back(&*token_view.begin(), &*token_view.end());
      }
    }

    if (tokens.empty() || tokens[0].empty() || tokens[0][0] == '#') {
      continue;
    }

    if (tokens[0] == "v") {
      if (tokens.size() < 4)
        continue;
      if (auto x = try_parse_float(tokens[1]),
          y = try_parse_float(tokens[2]),
          z = try_parse_float(tokens[3]);
          x && y && z) {
        positions_storage.push_back({ *x, *y, *z });
      }
    } else if (tokens[0] == "vt") {
      if (tokens.size() < 3)
        continue;
      if (auto x = try_parse_float(tokens[1]), y = try_parse_float(tokens[2]);
          x && y) {
        tex_coords_storage.push_back({ *x, *y });
      }
    } else if (tokens[0] == "vn") {
      if (tokens.size() < 4)
        continue;
      if (auto x = try_parse_float(tokens[1]),
          y = try_parse_float(tokens[2]),
          z = try_parse_float(tokens[3]);
          x && y && z) {
        normals_storage.push_back({ *x, *y, *z });
      }
    } else if (tokens[0] == "f") {
      if (tokens.size() < 4)
        continue;

      std::vector<GLuint> face_corner_indices;
      for (size_t i = 1; i < tokens.size(); ++i) {
        face_vertex_tokens.clear();
        for (const auto idx_token :
             tokens[i] | std::ranges::views::split('/')) {

          face_vertex_tokens.emplace_back(&*idx_token.begin(),
                                          &*idx_token.end());
        }

        std::optional<int> opt_pos_idx, opt_tex_idx, opt_norm_idx;

        if (!face_vertex_tokens.empty())
          opt_pos_idx = parse_and_validate_idx(face_vertex_tokens[0],
                                               positions_storage.size());
        if (face_vertex_tokens.size() > 1 && !face_vertex_tokens[1].empty())
          opt_tex_idx = parse_and_validate_idx(face_vertex_tokens[1],
                                               tex_coords_storage.size());
        if (face_vertex_tokens.size() > 2 && !face_vertex_tokens[2].empty())
          opt_norm_idx = parse_and_validate_idx(face_vertex_tokens[2],
                                                normals_storage.size());

        if (!opt_pos_idx)
          continue;

        VertexKey key;
        key.pos_idx = *opt_pos_idx;
        if constexpr (requires { Vertex::tex_coords; }) {
          if (opt_tex_idx)
            key.tex_idx = *opt_tex_idx;
        }
        if constexpr (requires { Vertex::normal; }) {
          if (opt_norm_idx)
            key.norm_idx = *opt_norm_idx;
        }

        if (auto it = unique_vertices.find(key); it != unique_vertices.end()) {
          face_corner_indices.push_back(it->second);
        } else {
          Vertex new_vertex;
          if constexpr (requires { new_vertex.position; }) {
            new_vertex.position = positions_storage[key.pos_idx];
          }
          if constexpr (requires { new_vertex.tex_coords; }) {
            if (key.tex_idx != -1)
              new_vertex.tex_coords = tex_coords_storage[key.tex_idx];
          }
          if constexpr (requires { new_vertex.normal; }) {
            if (key.norm_idx != -1)
              new_vertex.normal = normals_storage[key.norm_idx];
          }

          final_vertices.push_back(new_vertex);
          GLuint new_idx = static_cast<GLuint>(final_vertices.size() - 1);
          unique_vertices[key] = new_idx;
          face_corner_indices.push_back(new_idx);
        }
      }

      if (face_corner_indices.size() >= 3) {
        for (size_t i = 1; i < face_corner_indices.size() - 1; ++i) {
          final_indices.push_back({ face_corner_indices[0],
                                    face_corner_indices[i],
                                    face_corner_indices[i + 1] });
        }
      }
    }
  }

  return {path, std::move(final_vertices), std::move(final_indices), draw_primitive, usage};
}
