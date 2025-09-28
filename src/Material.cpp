#include "Material.h"
#include "Repo.h"
#include "Shader.h"
#include <algorithm>
#include <memory>

int Material::m_id = 0;
int Material::m_current_texture_slot = 0;

int
Material::get_id()
{
  return ++m_id;
}

void
Material::load(TextureRepo& tex_repo,
               std::shared_ptr<ShaderProgram> shader_program,
               const std::vector<Texture>& textures)
{
  m_tex_repo = &tex_repo;
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
  if (m_tex_repo == nullptr) {
    return;
  }

  for (const auto& texture : m_textures) {
    glBindTextureUnit(get_texture_slot(texture.name),
                      m_tex_repo->get_texture(texture.name, texture.desc));
  }
}

void
Material::unbind()
{
  m_current_texture_slot = 0;
}
