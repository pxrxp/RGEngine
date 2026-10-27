#pragma once

#include "repos/MaterialRepo.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

class ShaderProgram;
class Texture;

using ShaderProgramStrongPtr = std::shared_ptr<ShaderProgram>;
using TextureStrongPtr = std::shared_ptr<Texture>;

using TextureName = std::string;
using UniformValue = std::variant<int,
                                  float,
                                  glm::vec2,
                                  glm::vec3,
                                  glm::vec4,
                                  glm::mat3,
                                  glm::mat4,
                                  TextureName>;

class Material
{
private:
  static int m_id_counter;
  int m_id;
  MaterialRepo& m_repo_cache;
  std::string m_name;
  ShaderProgramStrongPtr m_shader_program;
  std::vector<TextureStrongPtr> m_textures;
  std::unordered_map<std::string, UniformValue> m_uniforms;

  // Only deactivate slots this material successfully acquired during bind().
  // Calling deactivate_slot() for a texture that never got a slot can crash
  // MaterialRepo implementations that assume the slot exists.
  std::unordered_set<TextureName> m_active_texture_names;

public:
  Material(MaterialRepo& repo, const std::string& material_name, ShaderProgramStrongPtr shader_program,
           const std::vector<TextureStrongPtr>& textures,
           const std::unordered_map<std::string, UniformValue>& uniforms = {})
    : m_id(++m_id_counter)
    , m_repo_cache(repo)
    , m_name(material_name)
    , m_shader_program(shader_program)
    , m_textures(textures)
    , m_uniforms(uniforms)
  {
  }

  ShaderProgramStrongPtr get_shader_program() const { return m_shader_program; }

  int get_id() { return m_id; }
  const std::string& get_name() { return m_name; }
  
  void bind();
  void unbind();
};
