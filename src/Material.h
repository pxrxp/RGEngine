#pragma once
#include "opengl.h"
#include <memory>
#include <string>
#include <vector>

class ShaderProgram;

struct TextureDescriptor
{
  GLenum target = GL_TEXTURE_2D;
  GLenum internal_format = GL_RGBA8;
  bool generate_mipmaps = true;
  GLenum min_filter = GL_LINEAR_MIPMAP_LINEAR;
  GLenum mag_filter = GL_LINEAR;
  GLenum wrap_s = GL_REPEAT;
  GLenum wrap_t = GL_REPEAT;
  GLenum wrap_r = GL_CLAMP_TO_EDGE;
  int layers = -1; // ONLY DEFINED FOR GL_TEXTURE_2D_ARRAY
};

struct Texture
{
  std::string name;
  TextureDescriptor desc;
};

inline TextureDescriptor DEFAULT_TEXTURE_DESCRIPTOR;

class Material
{
private:
  static int m_id, m_current_texture_slot;
  int m_texture_slot;
  std::vector<Texture> m_textures;

public:
  int get_id();
  void load(std::shared_ptr<ShaderProgram> shader_program,
            const std::vector<Texture>& textures);
  int get_texture_slot(const std::string& texture);
  void bind();
  void unbind();
};

inline int Material::m_id = 0;
inline int Material::m_current_texture_slot = 0;
