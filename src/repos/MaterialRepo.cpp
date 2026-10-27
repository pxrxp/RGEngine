#include "MaterialRepo.h"
#include "../Material.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <utility>

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
    if (!desc.is_getter_desc())
      std::cout << "Material " << desc.material_name << " already exists!"
                << std::endl;
    return material.value();
  }

  std::vector<TextureStrongPtr> textures;
  for (auto& desc : desc.texture_desc_list) {
    textures.push_back(tex_repo.load_texture(desc));
  }

  auto shader_program = shader_program_repo.load_program(desc.shader_program_desc);
  if (!shader_program)
    return {};

  return RepoBase::create(desc.material_name,
                          *this,
                          desc.material_name,
                          std::move(shader_program),
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
  const auto it = texture_slots_map.find(texture_name);
  if (it == texture_slots_map.end()) {
    std::cerr << "MaterialRepo::activate_slot(): no slot assigned for '"
              << texture_name << "'\n";
    return;
  }

  const unsigned int slot = it->second;
  empty_texture_slots.erase(slot);
  full_texture_slots.insert(slot);
}

void
MaterialRepo::deactivate_slot(const std::string& texture_name)
{
  const auto it = texture_slots_map.find(texture_name);
  if (it == texture_slots_map.end()) {
    // A material may have failed to acquire a slot during bind(). Nothing to
    // release; importantly, do not dereference end() or alter another slot.
    return;
  }

  const unsigned int slot = it->second;
  full_texture_slots.erase(slot);
  empty_texture_slots.insert(slot);
}
