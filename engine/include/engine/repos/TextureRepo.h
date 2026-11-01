#pragma once
#include <engine/repos/RepoBase.h>
#include <engine/util/opengl.h>
#include <compare>
#include <memory>
#include <string>

class Texture;
using TextureStrongPtr = std::shared_ptr<Texture>;
using TextureWeakPtr = std::weak_ptr<Texture>;

struct TextureDescriptor
{
  // Repository/cache key. Existing callers can keep using a base name such as "ground".
  std::string texture_name;
  GLenum target = GL_TEXTURE_2D;
  GLenum internal_format = GL_RGBA8;
  GLenum wrap_s = GL_REPEAT;
  GLenum wrap_t = GL_REPEAT;
  GLenum wrap_r = GL_CLAMP_TO_EDGE;
  GLenum min_filter = GL_LINEAR_MIPMAP_LINEAR;
  GLenum mag_filter = GL_LINEAR;
  bool generate_mipmaps = true;
  int layers = -1;
  int max_mip_level = -1;

  auto operator<=>(const TextureDescriptor&) const = default;
  bool is_getter_desc() const {
    return *this == TextureDescriptor { .texture_name = this->texture_name };
  }
};

class TextureRepo : public RepoBase<std::string, Texture>
{
private:
  TextureStrongPtr load_tex_2d(const TextureDescriptor& desc);
  TextureStrongPtr load_tex_2d_array(const TextureDescriptor& desc);
  TextureStrongPtr load_cubemap(const TextureDescriptor& desc);

public:
  TextureStrongPtr load_texture(const TextureDescriptor& desc);
};
