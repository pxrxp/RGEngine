#include "MaterialRepo.h"
#include <cmath>
#include <iostream>

MaterialRepo::MaterialRepo() {
  GLint max_slots;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_slots);
  clear_all_slots();
}

void
MaterialRepo::clear_all_slots()
{
  texture_slots_map.clear();
  full_texture_slots.clear();

  GLint max_slots;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_slots);
  for (unsigned int i = 0; i < static_cast<unsigned int>(max_slots); i++) {
    empty_texture_slots.insert(i);
  }
}

MaterialStrongPtr
MaterialRepo::load_material(const MaterialDescriptor& desc)
{
  if (auto material = RepoBase::get(desc.material_name); material.has_value()) {
    std::cerr << "Material " << desc.material_name << " already exists!"
              << std::endl;
    return material.value();
  }

  std::vector<TextureStrongPtr> textures;
  for (auto& desc : desc.texture_desc_list) {
    textures.push_back(tex_repo.load_texture(desc));
  }

  return RepoBase::create(
    desc.material_name,
    *this,
    desc.material_name,
    shader_program_repo.load_program(desc.shader_program_desc),
    textures,
    desc.uniforms);
}

unsigned int
MaterialRepo::assign_slot(const std::string& texture_name)
{
  if (auto it = texture_slots_map.find(texture_name); it != texture_slots_map.end())
    return it->second;
  
  auto slot = *empty_texture_slots.begin();
  empty_texture_slots.erase(slot);
  full_texture_slots.insert(slot);
  return slot;
}

void
MaterialRepo::empty_slot(unsigned int slot)
{
  full_texture_slots.erase(slot);
  empty_texture_slots.insert(slot);
}

void MaterialRepo::activate_slot(const std::string& texture_name) {
}
