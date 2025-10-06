#include "Material.h"
#include "util/opengl.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include <iostream>

int Material::m_id_counter = -1;

void
Material::bind()
{
  for (const auto& [uniform_name, uniform_value] : m_uniforms) {
    std::visit(
      [&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, TextureName>) {
          const TextureName& tex_name_to_find = arg;

          for (auto& texture_ptr : m_textures) {
            if (texture_ptr && texture_ptr->get_name() == tex_name_to_find) {
              if (auto slot = m_repo_cache.assign_slot(texture_ptr->get_name());
                  slot.has_value()) {
                glBindTextureUnit(slot.value(), texture_ptr->get_id());
                m_shader_program->set_uniform(uniform_name.c_str(),
                                              static_cast<GLint>(slot.value()));
                break;
              }
            }
          }
        } else {
          m_shader_program->set_uniform(uniform_name.c_str(), arg);
        }
      },
      uniform_value);
  }
}

void
Material::unbind()
{
  for (auto& texture_ptr : m_textures) {
    m_repo_cache.deactivate_slot(texture_ptr->get_name());
  }
}
