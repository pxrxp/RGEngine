#include "Material.h"
#include "Repo.h"
#include "Shader.h"
#include <algorithm>
#include <memory>

#include "debug.h"

TextureRepo texture_repo;

int
Material::get_id()
{
  return ++m_id;
}

void
Material::load(std::shared_ptr<ShaderProgram> shader_program,
               const std::vector<Texture>& textures)
{
  m_textures = textures;
  m_texture_slot = m_current_texture_slot;
  for (const auto& texture : m_textures) {
    shader_program->set_uniform(texture.name.c_str(), m_current_texture_slot++);
  }
}

int
Material::get_texture_slot(const std::string& texture)
{
  auto it = std::find_if(m_textures.begin(),
                         m_textures.end(),
                         [&](const auto& x) { return x.name == texture; });

  if (it == m_textures.end())
    return -1;

  return m_current_texture_slot + std::distance(m_textures.begin(), it);
}

void
Material::bind()
{
  for (const auto& texture : m_textures) {
    glBindTextureUnit(get_texture_slot(texture.name),
                      texture_repo.get_texture(texture.name, texture.desc));
  }
}

void
Material::unbind()
{
  m_current_texture_slot = 0;
}
