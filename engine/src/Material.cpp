#include <engine/Material.h>
#include <engine/ShaderProgram.h>
#include <engine/Texture.h>
#include <engine/util/log.h>
#include <engine/util/opengl.h>
#include <iostream>
#include <type_traits>

int Material::m_id_counter = -1;

void
Material::bind()
{
  if (!m_shader_program) {
    Util::log_err() << "Material::bind(): material '" << m_name
                    << "' has no shader program; skipping bind\n";
    return;
  }

  for (const auto& [uniform_name, uniform_value] : m_uniforms) {
    std::visit(
      [&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, TextureName>) {
          const TextureName& requested_name = arg;
          bool found_texture = false;

          for (const auto& texture_ptr : m_textures) {
            if (!texture_ptr || texture_ptr->get_name() != requested_name) {
              continue;
            }

            found_texture = true;
            const TextureName actual_name = texture_ptr->get_name();
            if (auto slot = m_repo_cache.assign_slot(actual_name);
                slot.has_value()) {
              glBindTextureUnit(slot.value(), texture_ptr->get_id());
              m_shader_program->set_uniform(uniform_name.c_str(),
                                            static_cast<GLint>(slot.value()));
              m_active_texture_names.insert(actual_name);
            } else {
              Util::log_err()
                << "Material::bind(): no texture slot available for '"
                << actual_name << "' in material '" << m_name << "'\n";
            }
            break;
          }

          if (!found_texture) {
            Util::log_err()
              << "Material::bind(): material '" << m_name
              << "' has no loaded texture named '" << requested_name
              << "' for uniform '" << uniform_name << "'\n";
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
  // Never call deactivate_slot() for null, missing, or unassigned textures.
  // MaterialRepo::deactivate_slot() expects a slot to exist for the name.
  for (const auto& texture_name : m_active_texture_names) {
    m_repo_cache.deactivate_slot(texture_name);
  }
  m_active_texture_names.clear();
}
