#pragma once
#include "Repo.h"

class TextureRepo
{
private:
  std::unordered_map<std::string, GLuint> m_textures;
  GLuint load_and_store_texture(const std::string& names,
                                const TextureDescriptor& desc);
  GLuint load_and_store_tex_2d(const std::string& names,
                               const TextureDescriptor& desc);
  GLuint load_and_store_tex_2d_array(const std::string& names,
                                     const TextureDescriptor& desc);
  GLuint load_and_store_cubemap(const std::string& names,
                                const TextureDescriptor& desc);

public:
  GLuint get_texture(
    const std::string& name,
    const TextureDescriptor& desc = DEFAULT_TEXTURE_DESCRIPTOR);

  void remove_texture(const std::string& name);
  void clear();
};
