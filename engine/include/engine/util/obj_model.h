#pragma once

#include <engine/components/vertex_formats.h>
#include <engine/repos/MeshRepo.h>
#include <engine/util/filesystem.h>

#include <charconv>
#include <cstdio>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include <cctype>
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
from_OBJ(const std::string& path,
         GLenum draw_primitive = GL_TRIANGLES,
         GLenum usage = GL_STATIC_DRAW)
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
        if constexpr (requires { Vertex::texture; }) {
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
          if constexpr (requires { new_vertex.texture; }) {
            if (key.tex_idx != -1)
              new_vertex.texture = glm::vec2(tex_coords_storage[key.tex_idx].x,
                                             1.0f - tex_coords_storage[key.tex_idx].y);
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

  return { path,
           std::move(final_vertices),
           std::move(final_indices),
           draw_primitive,
           usage };
}

struct ObjMtlMaterial
{
  std::string name;
  glm::vec3 diffuse{1.0f};
  float opacity = 1.0f;
  std::filesystem::path diffuse_texture;
};

template<typename Vertex, typename Indices>
struct ObjMaterialMeshGroup
{
  std::string material_name;
  std::filesystem::path diffuse_texture;
  glm::vec3 diffuse{1.0f};
  float opacity = 1.0f;
  MeshDescriptor<Vertex, Indices> mesh;
};

namespace obj_mtl_detail {
inline std::string trim(std::string s)
{
  const auto first = s.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return {};
  const auto last = s.find_last_not_of(" \t\r\n");
  return s.substr(first, last - first + 1);
}

inline std::filesystem::path resource_path(const std::string& path)
{
  std::filesystem::path p(path);
  if (p.is_absolute() || std::filesystem::exists(p)) return p;
  return std::filesystem::path("resources") / p;
}

inline std::filesystem::path resolve_map_path(
  const std::filesystem::path& mtl_path, const std::string& raw_path)
{
  auto raw = std::filesystem::path(trim(raw_path));
  if (raw.empty()) return {};
  if (raw.is_absolute() && std::filesystem::exists(raw)) return raw;

  const auto base = mtl_path.parent_path();
  auto candidate = (base / raw).lexically_normal();
  if (std::filesystem::exists(candidate)) return candidate;

  auto res_cand = (std::filesystem::path("resources") / raw).lexically_normal();
  if (std::filesystem::exists(res_cand)) return res_cand;

  auto fn_cand = (base / raw.filename()).lexically_normal();
  if (std::filesystem::exists(fn_cand)) return fn_cand;

  return candidate;
}

inline std::unordered_map<std::string, ObjMtlMaterial>
read_mtl(const std::filesystem::path& mtl_path)
{
  std::unordered_map<std::string, ObjMtlMaterial> materials;
  std::ifstream file(mtl_path);
  if (!file) {
    std::fprintf(stderr, "OBJ loader: could not open MTL '%s'\n", mtl_path.string().c_str());
    return materials;
  }
  ObjMtlMaterial* current = nullptr;
  std::string line;
  while (std::getline(file, line)) {
    line = trim(std::move(line));
    if (line.empty() || line[0] == '#') continue;
    std::istringstream in(line);
    std::string command;
    in >> command;
    if (command == "newmtl") {
      std::string name;
      std::getline(in, name);
      name = trim(std::move(name));
      auto [it, inserted] = materials.try_emplace(name);
      it->second.name = name;
      current = &it->second;
    } else if (current && command == "Kd") {
      in >> current->diffuse.r >> current->diffuse.g >> current->diffuse.b;
    } else if (current && (command == "d" || command == "Tr")) {
      float value = 1.0f;
      if (in >> value) current->opacity = command == "Tr" ? 1.0f - value : value;
    } else if (current && command == "map_Kd") {
      std::string path;
      std::getline(in, path);
      path = trim(std::move(path));
      while (!path.empty() && path[0] == '-') {
        std::istringstream option_stream(path);
        std::string option;
        option_stream >> option;
        std::string rest;
        std::getline(option_stream, rest);
        rest = trim(std::move(rest));
        if (option == "-s" || option == "-o" || option == "-t") {
          std::istringstream values(rest);
          float ignored; int count = 0;
          while (count < 3 && values >> ignored) ++count;
          std::string remaining;
          std::getline(values, remaining);
          path = trim(std::move(remaining));
        } else {
          const auto split = rest.find_first_of(" \t");
          path = split == std::string::npos ? std::string{} : trim(rest.substr(split + 1));
        }
      }
      current->diffuse_texture = resolve_map_path(mtl_path, path);
    }
  }
  return materials;
}
}

template<typename Vertex, typename Indices>
std::vector<ObjMaterialMeshGroup<Vertex, Indices>>
from_OBJ_material_groups(const std::string& path,
                         GLenum draw_primitive = GL_TRIANGLES,
                         GLenum usage = GL_STATIC_DRAW)
{
  static_assert(std::is_same<Vertex, Vertex_Pos>::value ||
                std::is_same<Vertex, Vertex_PosCol>::value ||
                std::is_same<Vertex, Vertex_PosTex>::value ||
                std::is_same<Vertex, Vertex_PosColNorm>::value ||
                std::is_same<Vertex, Vertex_PosTexNorm>::value ||
                std::is_same<Vertex, Vertex_PosTexNormTanBitan>::value);

  namespace fs = std::filesystem;
  const fs::path obj_path = obj_mtl_detail::resource_path(path);
  std::ifstream obj_file(obj_path);
  if (!obj_file) {
    std::fprintf(stderr, "OBJ loader: could not open OBJ '%s'\n", obj_path.string().c_str());
    return {};
  }

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> normals;
  std::vector<fs::path> mtl_paths;
  std::vector<std::string> material_order;
  std::unordered_map<std::string, ObjMtlMaterial> materials;
  std::string active_material = "__default__";

  struct Builder {
    std::vector<Vertex> vertices;
    std::vector<Indices> indices;
    std::unordered_map<VertexKey, GLuint, VertexKeyHasher> unique;
  };
  std::unordered_map<std::string, Builder> builders;
  auto ensure_group = [&](const std::string& name) {
    if (!builders.contains(name)) material_order.push_back(name);
    builders.try_emplace(name);
  };
  ensure_group(active_material);

  std::string line;
  while (std::getline(obj_file, line)) {
    line = obj_mtl_detail::trim(std::move(line));
    if (line.empty() || line[0] == '#') continue;
    std::istringstream in(line);
    std::string command;
    in >> command;
    if (command == "v") {
      glm::vec3 v;
      if (in >> v.x >> v.y >> v.z) positions.push_back(v);
    } else if (command == "vt") {
      glm::vec2 v;
      if (in >> v.x >> v.y) texcoords.push_back(v);
    } else if (command == "vn") {
      glm::vec3 v;
      if (in >> v.x >> v.y >> v.z) normals.push_back(v);
    } else if (command == "mtllib") {
      std::string mtl_name;
      while (in >> mtl_name) mtl_paths.push_back((obj_path.parent_path() / mtl_name).lexically_normal());
    } else if (command == "usemtl") {
      std::getline(in, active_material);
      active_material = obj_mtl_detail::trim(std::move(active_material));
      if (active_material.empty()) active_material = "__default__";
      ensure_group(active_material);
    } else if (command == "f") {
      std::string corner;
      auto& builder = builders[active_material];
      std::vector<GLuint> corners;
      while (in >> corner) {
        const auto first = corner.find('/');
        const auto second = first == std::string::npos ? std::string::npos : corner.find('/', first + 1);
        auto parse_index = [](std::string_view sv, size_t count) -> std::optional<int> {
          if (sv.empty()) return std::nullopt;
          int raw = 0;
          auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), raw);
          if (ec != std::errc() || ptr != sv.data() + sv.size() || raw == 0) return std::nullopt;
          const int index = raw > 0 ? raw - 1 : static_cast<int>(count) + raw;
          if (index < 0 || static_cast<size_t>(index) >= count) return std::nullopt;
          return index;
        };
        auto op = parse_index(first == std::string::npos ? std::string_view(corner) : std::string_view(corner).substr(0, first), positions.size());
        if (!op) continue;
        std::optional<int> ot, on;
        if (first != std::string::npos) {
          const auto tex_end = second == std::string::npos ? corner.size() : second;
          ot = parse_index(std::string_view(corner).substr(first + 1, tex_end - first - 1), texcoords.size());
          if (second != std::string::npos) on = parse_index(std::string_view(corner).substr(second + 1), normals.size());
        }
        VertexKey key; key.pos_idx = *op;
        if constexpr (requires { Vertex::texture; }) { if (ot) key.tex_idx = *ot; }
        if constexpr (requires { Vertex::normal; }) { if (on) key.norm_idx = *on; }
        auto found = builder.unique.find(key);
        if (found != builder.unique.end()) corners.push_back(found->second);
        else {
          Vertex v{};
          if constexpr (requires { v.position; }) v.position = positions[key.pos_idx];
          if constexpr (requires { v.texture; }) {
            if (key.tex_idx >= 0) {
              v.texture = glm::vec2(texcoords[key.tex_idx].x, 1.0f - texcoords[key.tex_idx].y);
            }
          }
          if constexpr (requires { v.normal; }) if (key.norm_idx >= 0) v.normal = normals[key.norm_idx];
          builder.vertices.push_back(v);
          const auto index = static_cast<GLuint>(builder.vertices.size() - 1);
          builder.unique.emplace(key, index);
          corners.push_back(index);
        }
      }
      for (size_t i = 1; i + 1 < corners.size(); ++i)
        builder.indices.push_back({ corners[0], corners[i], corners[i + 1] });
    }
  }

  for (const auto& mtl_path : mtl_paths) {
    auto loaded = obj_mtl_detail::read_mtl(mtl_path);
    for (auto& [name, material] : loaded) materials[name] = std::move(material);
  }

  std::vector<ObjMaterialMeshGroup<Vertex, Indices>> result;
  result.reserve(material_order.size());
  for (const auto& name : material_order) {
    auto it = builders.find(name);
    if (it == builders.end() || it->second.indices.empty()) continue;
    ObjMtlMaterial material;
    material.name = name;
    if (auto mat = materials.find(name); mat != materials.end()) material = mat->second;
    MeshDescriptor<Vertex, Indices> desc{
      path + "#" + name,
      std::move(it->second.vertices),
      std::move(it->second.indices),
      draw_primitive,
      usage
    };
    result.push_back({ name, material.diffuse_texture, material.diffuse, material.opacity, std::move(desc) });
  }
  return result;
}