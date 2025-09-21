#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include <memory>

class Texture;
class ShaderProgram;

int Material::get_id() {
  return this->id++;
}

// void Material::add(std::shared_ptr<Texture> texture) {
  
// }

void Material::bind(std::shared_ptr<ShaderProgram> shader_program) {
  // shader_program->bind();
  // for (const auto& texture : textures) {
  //   shader_program->set_texture_unit(texture->get_name(), texture->get_texture_unit());
  // }
  // shader_program->return_back();
}
