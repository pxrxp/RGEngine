#include "MaterialRepo.h"
#include "Material.h"
#include <cmath>
#include <iostream>
#include <vector>

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
    // If only material_name provided, the user wants to access it only => no
    // error
    if (desc != MaterialDescriptor{ desc.material_name })
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

std::optional<unsigned int>
MaterialRepo::assign_slot(const std::string& texture_name)
{
  unsigned int slot;
  if (auto it = texture_slots_map.find(texture_name);
      it != texture_slots_map.end()) {
    slot = it->second;
  } else if (auto it = empty_texture_slots.begin();
             it != empty_texture_slots.end()) {
    slot = *empty_texture_slots.begin();
    empty_texture_slots.erase(slot);
    full_texture_slots.insert(slot);

    std::erase_if(texture_slots_map,
                  [&](const auto& pair) { return pair.second == slot; });

    texture_slots_map[texture_name] = slot;
  } else {
    std::cerr << "No empty slots for " << texture_name << std::endl;
    return std::nullopt;
  }

  activate_slot(slot);
  return slot;
}

void
MaterialRepo::activate_slot(unsigned int slot)
{
  empty_texture_slots.erase(slot);
  full_texture_slots.insert(slot);
}

void
MaterialRepo::deactivate_slot(unsigned int slot)
{
  full_texture_slots.erase(slot);
  empty_texture_slots.insert(slot);
}

void
MaterialRepo::activate_slot(const std::string& texture_name)
{
  auto it = texture_slots_map.find(texture_name);
  if (it == texture_slots_map.end())
    std::cout << "Slot not assigned for " << texture_name << std::endl;

  unsigned int slot = it->second;
  empty_texture_slots.erase(slot);
  full_texture_slots.insert(slot);
}

void
MaterialRepo::deactivate_slot(const std::string& texture_name)
{
  auto it = texture_slots_map.find(texture_name);
  if (it == texture_slots_map.end())
    std::cout << "Slot not assigned for " << texture_name << std::endl;

  unsigned int slot = it->second;

  empty_texture_slots.erase(slot);
  full_texture_slots.insert(slot);
}
