#pragma once
#include "TextureRepo.h"
#include "TextureRepo.h"
#include <string>

class MaterialRepo : public RepoBase<std::string, Material>
{
private:
  TextureRepo tex_repo;
  int m_id = 0;
  int m_current_texture_slot = 0;
  
public:
  void
  load_material(
                 std::shared_ptr<ShaderProgram> shader_program,
                 const std::vector<Texture>& textures)
  {
    int m_texture_slot = m_current_texture_slot;
    for (const auto& texture : textures) {
      shader_program->set_uniform(texture.name.c_str(), m_current_texture_slot++);
    }
  }

  // int
  // get_texture_slot(const std::string& texture)
  // {
  //   auto it = std::find_if(m_textures.begin(),
  //                          m_textures.end(),
  //                          [&](const auto& x) { return x.name == texture; });

  //   if (it == m_textures.end())
  //     return -1;

  //   return m_current_texture_slot + std::distance(m_textures.begin(), it);
  // }
  
};
