#pragma once
#include "RepoBase.h"
#include "ShaderProgramRepo.h"
#include "TextureRepo.h"
#include <glm/glm.hpp>
#include <set>
#include <unordered_map>
#include <variant>
#include <vector>

class Material;
using MaterialStrongPtr = std::shared_ptr<Material>;
using MaterialWeakPtr = std::weak_ptr<Material>;

using TextureName = std::string;
using UniformValue = std::variant<int,
                                  float,
                                  glm::vec2,
                                  glm::vec3,
                                  glm::vec4,
                                  glm::mat3,
                                  glm::mat4,
                                  TextureName>;

struct MaterialDescriptor
{
  std::string material_name;
  ShaderProgramDescriptor shader_program_desc;
  std::vector<TextureDescriptor> texture_desc_list;
  std::unordered_map<std::string, UniformValue> uniforms;

  bool is_getter_desc() const
  {
    return shader_program_desc.is_getter_desc() && texture_desc_list.empty() &&
           uniforms.empty();
  }
};

class MaterialRepo : public RepoBase<std::string, Material>
{
private:
  std::unordered_map<std::string, GLuint> texture_slots_map;
  std::set<unsigned int> full_texture_slots;
  std::set<unsigned int> empty_texture_slots;

public:
  TextureRepo tex_repo;
  ShaderProgramRepo shader_program_repo;

  void clear_all_slots();

  MaterialStrongPtr load_material(const MaterialDescriptor& desc);
  std::optional<unsigned int> assign_slot(const std::string& texture_name);
  void activate_slot(unsigned int slot);
  void activate_slot(const std::string& texture_name);
  void deactivate_slot(unsigned int slot);
  void deactivate_slot(const std::string& texture_name);
};
