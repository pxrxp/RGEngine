#pragma once
#include "opengl.h"
#include <memory>
#include <vector>
#include <string>

class ShaderProgram;
struct Texture {
  std::string name;
  GLenum target;
};

class Material {
private:
  static int m_id, m_current_texture_slot;
  int m_texture_slot;
  std::vector<Texture> m_textures;

public:
  int get_id();
  void load(std::shared_ptr<ShaderProgram> shader_program,
            const std::vector<Texture>&textures);
  int get_texture_slot(const std::string &texture);
  void bind();
  void unbind();
};

inline int Material::m_id = 0;
inline int Material::m_current_texture_slot = 0;
