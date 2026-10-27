#include "TextureRepo.h"
#include "../Texture.h"
#include "../util/png_image.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

namespace {
int
calculate_mip_levels(int width, int height, int depth = 1)
{
  return 1 + static_cast<int>(
               std::floor(std::log2(std::max({ width, height, depth }))));
}

std::pair<GLenum, GLenum>
fix_filter(bool generate_mipmaps, GLenum min_filter, GLenum mag_filter)
{
  GLenum _min_filter = min_filter, _mag_filter = mag_filter;
  if (generate_mipmaps) {
    if (min_filter == GL_LINEAR)
      _min_filter = GL_LINEAR_MIPMAP_LINEAR;
    if (min_filter == GL_NEAREST)
      _min_filter = GL_NEAREST_MIPMAP_NEAREST;
  } else {
    if (min_filter == GL_NEAREST_MIPMAP_NEAREST ||
        min_filter == GL_NEAREST_MIPMAP_LINEAR)
      _min_filter = GL_NEAREST;
    if (min_filter == GL_LINEAR_MIPMAP_LINEAR ||
        min_filter == GL_LINEAR_MIPMAP_NEAREST)
      _min_filter = GL_LINEAR;
  }
  if (mag_filter == GL_NEAREST_MIPMAP_NEAREST ||
      mag_filter == GL_NEAREST_MIPMAP_LINEAR)
    _mag_filter = GL_NEAREST;
  if (mag_filter == GL_LINEAR_MIPMAP_LINEAR ||
      mag_filter == GL_LINEAR_MIPMAP_NEAREST)
    _mag_filter = GL_LINEAR;
  return { _min_filter, _mag_filter };
}

}

TextureStrongPtr
TextureRepo::load_texture(const TextureDescriptor& desc)
{
  if (desc.texture_name.empty()) {
    std::cerr << "Provide at least one texture!" << std::endl;
    return nullptr;
  }
  if (auto tex = RepoBase::get(desc.texture_name); tex.has_value()) {
    if (!desc.is_getter_desc())
      std::cout << "Texture " << desc.texture_name << " already exists!"
                << std::endl;
    return tex.value();
  }

  TextureStrongPtr tex = nullptr;
  switch (desc.target) {
    case GL_TEXTURE_2D:
      tex = load_tex_2d(desc);
      break;
    case GL_TEXTURE_CUBE_MAP:
      tex = load_cubemap(desc);
      break;
    case GL_TEXTURE_2D_ARRAY:
      tex = load_tex_2d_array(desc);
      break;
    default:
      std::cerr << "Invalid target! Expected GL_TEXTURE_2D | "
                   "GL_TEXTURE_CUBE_MAP | GL_TEXTURE_2D_ARRAY\n";
      break;
  }
  if (tex)
    m_resources[desc.texture_name] = tex;
  return tex;
}

TextureStrongPtr
TextureRepo::load_tex_2d(const TextureDescriptor& desc)
{
  PNGImage image(desc.texture_name + ".png");
  if (!image.is_valid()) {
    std::cerr << "ERROR: Invalid file " << desc.texture_name + ".png\n";
    return nullptr;
  }
  GLuint texture = 0;
  glCreateTextures(desc.target, 1, &texture);
  const GLenum source_format = (image.channels == 4) ? GL_RGBA : GL_RGB;
  const int levels = desc.generate_mipmaps
                       ? calculate_mip_levels(static_cast<int>(image.width),
                                              static_cast<int>(image.height))
                       : 1;
  glTextureStorage2D(
    texture, levels, desc.internal_format, image.width, image.height);
  glTextureSubImage2D(texture,
                      0,
                      0,
                      0,
                      image.width,
                      image.height,
                      source_format,
                      GL_UNSIGNED_BYTE,
                      image.pixels.data());
  if (desc.generate_mipmaps)
    glGenerateTextureMipmap(texture);
  const auto filters =
    fix_filter(desc.generate_mipmaps, desc.min_filter, desc.mag_filter);
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, filters.first);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, filters.second);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, desc.wrap_s);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, desc.wrap_t);
  if (desc.max_mip_level >= 0)
    glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, desc.max_mip_level);
  return RepoBase::create(
    desc.texture_name, texture, desc.texture_name, image.width, image.height);
}

TextureStrongPtr
TextureRepo::load_tex_2d_array(const TextureDescriptor& desc)
{
  PNGImage image(desc.texture_name + "_0.png");
  if (!image.is_valid()) {
    std::cerr << "ERROR: Invalid file " << desc.texture_name + "_0.png\n";
    return nullptr;
  }
  if (image.width != image.height) {
    std::cerr << "ERROR: Texture array source images must be square!\n";
    return nullptr;
  }
  if (desc.layers <= 0) {
    std::cerr << "ERROR: Invalid number of texture array layers!\n";
    return nullptr;
  }
  const std::uint32_t width = image.width;
  GLuint texture = 0;
  glCreateTextures(desc.target, 1, &texture);
  const int levels = desc.generate_mipmaps
                       ? calculate_mip_levels(static_cast<int>(image.width),
                                              static_cast<int>(image.height))
                       : 1;
  glTextureStorage3D(texture,
                     levels,
                     desc.internal_format,
                     image.width,
                     image.height,
                     desc.layers);
  bool have_image = false;
  for (int i = 0; i < desc.layers; ++i) {
    const std::string file_name =
      desc.texture_name + "_" + std::to_string(i) + ".png";
    if (i > 0)
      image = PNGImage(file_name);
    if (!image.is_valid()) {
      std::cerr << "ERROR: Invalid file " << file_name << '\n';
      continue;
    }
    if (image.width != width || image.height != width) {
      std::cerr << "ERROR: Texture array layer " << i
                << " has mismatched dimensions.\n";
      continue;
    }
    const GLenum format = (image.channels == 4) ? GL_RGBA : GL_RGB;
    glTextureSubImage3D(texture,
                        0,
                        0,
                        0,
                        i,
                        width,
                        width,
                        1,
                        format,
                        GL_UNSIGNED_BYTE,
                        image.pixels.data());
    have_image = true;
  }
  if (!have_image) {
    glDeleteTextures(1, &texture);
    return nullptr;
  }
  if (desc.generate_mipmaps)
    glGenerateTextureMipmap(texture);
  const auto filters =
    fix_filter(desc.generate_mipmaps, desc.min_filter, desc.mag_filter);
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, filters.first);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, filters.second);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, desc.wrap_s);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, desc.wrap_t);
  return RepoBase::create(desc.texture_name, texture, desc.texture_name);
}

TextureStrongPtr
TextureRepo::load_cubemap(const TextureDescriptor& desc)
{
  const std::array<std::string, 6> suffixes = { "_right.png", "_left.png",
                                                "_top.png",   "_bottom.png",
                                                "_front.png", "_back.png" };
  PNGImage image(desc.texture_name + suffixes[0]);
  if (!image.is_valid()) {
    std::cerr << "ERROR: Invalid file " << desc.texture_name + suffixes[0]
              << '\n';
    return nullptr;
  }
  const std::uint32_t width = image.width, height = image.height;
  if (width != height) {
    std::cerr << "ERROR: Cubemap faces must be square!\n";
    return nullptr;
  }
  GLuint texture = 0;
  glCreateTextures(desc.target, 1, &texture);
  const int levels =
    desc.generate_mipmaps
      ? calculate_mip_levels(static_cast<int>(width), static_cast<int>(height))
      : 1;
  glTextureStorage2D(texture, levels, desc.internal_format, width, height);
  for (int i = 0; i < 6; ++i) {
    if (i > 0)
      image = PNGImage(desc.texture_name + suffixes[i]);
    if (!image.is_valid() || image.width != width || image.height != height) {
      std::cerr << "ERROR: Invalid/mismatched cubemap face "
                << desc.texture_name + suffixes[i] << '\n';
      glDeleteTextures(1, &texture);
      return nullptr;
    }
    const GLenum format = (image.channels == 4) ? GL_RGBA : GL_RGB;
    // A cubemap is six 2D faces, not one GL_TEXTURE_CUBE_MAP 2D image.
    // Upload each image to its corresponding face target.
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0,
                    0,
                    0,
                    width,
                    height,
                    format,
                    GL_UNSIGNED_BYTE,
                    image.pixels.data());
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }
  if (desc.generate_mipmaps)
    glGenerateTextureMipmap(texture);
  const auto filters =
    fix_filter(desc.generate_mipmaps, desc.min_filter, desc.mag_filter);
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, filters.first);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, filters.second);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, desc.wrap_s);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, desc.wrap_t);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_R, desc.wrap_r);
  return RepoBase::create(desc.texture_name, texture, desc.texture_name);
}
