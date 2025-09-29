#include "Material.h"
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

          for (size_t i = 0; i < m_textures.size(); i++) {
            auto texture_ptr = m_textures[i];
            auto slot = m_repo_cache.assign_slot(texture_ptr->get_name());
            if (texture_ptr && texture_ptr->get_name() == tex_name_to_find) {
              m_shader_program->set_uniform(uniform_name.c_str(),
                                            static_cast<GLint>(slot));

              break;
            }
          }
        } else {
          m_shader_program->set_uniform(uniform_name.c_str(), arg);
        }
      },
      uniform_value);
  }
}

void Material::unbind() {
  m_repo_cache.clear_all_slots();
}
