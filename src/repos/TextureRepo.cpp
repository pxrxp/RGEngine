#include "TextureRepo.h"
#include "../Texture.h"
#include "../util/png_image.h"
#include <algorithm>
#include <cmath>
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
      std::cerr << "Texture " << desc.texture_name << " already exists!" << std::endl;
    return tex.value();
  }

  std::shared_ptr<Texture> tex = nullptr;

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
      std::cerr
        << "Invalid target! Expected GL_TEXTURE_2D | GL_TEXTURE_CUBE_MAP "
           "| GL_TEXTURE_2D_ARRAY\n";
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
    std::cerr << "ERROR: Invalid file" << desc.texture_name + ".png\n";
  }

  GLuint texture;
  glCreateTextures(desc.target, 1, &texture);

  GLenum source_format = (image.channels == 4) ? GL_RGBA : GL_RGB;
  int levels =
    desc.generate_mipmaps ? calculate_mip_levels(image.width, image.height) : 1;

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

  auto fixed_filter =
    fix_filter(desc.generate_mipmaps, desc.min_filter, desc.mag_filter);
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, fixed_filter.first);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, fixed_filter.second);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, desc.wrap_s);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, desc.wrap_t);

  return RepoBase::create(desc.texture_name, texture, desc.texture_name);
}

std::shared_ptr<Texture>
TextureRepo::load_tex_2d_array(const TextureDescriptor& desc)
{
  PNGImage image = PNGImage(desc.texture_name + "_0.png");
  if (!image.is_valid()) {
    std::cerr << "ERROR: Invalid file" << desc.texture_name + "_0.png\n";
    return 0;
  }

  if (image.width != image.height) {
    std::cerr << "ERROR: Cubemap faces must be square!" << std::endl;
    return 0;
  }

  if (desc.layers == -1) {
    std::cerr << "ERROR: Invalid no. of layers!" << std::endl;
  }

  const auto width = image.width;

  GLuint texture;
  glCreateTextures(desc.target, 1, &texture);

  int levels =
    desc.generate_mipmaps ? calculate_mip_levels(image.width, image.height) : 1;

  glTextureStorage3D(texture,
                     levels,
                     desc.internal_format,
                     image.width,
                     image.height,
                     desc.layers);

  bool flag = false;

  for (int i = 0; i < desc.layers; ++i) {
    std::string file_name = desc.texture_name + "_" + std::to_string(i) + ".png";
    if (flag)
      image = PNGImage(file_name);

    if (!image.is_valid()) {
      std::cerr << "ERROR: Invalid file" << file_name << "\n";
      continue;
    }

    if (image.width != width || image.height != width) {
      std::cerr << "ERROR: Cubemap layer " << i << " has mismatched dimensions."
                << std::endl;
      continue;
    }

    GLenum sourceFormat = (image.channels == 4) ? GL_RGBA : GL_RGB;
    glTextureSubImage3D(texture,
                        0,
                        0,
                        0,
                        i,
                        width,
                        width,
                        1,
                        sourceFormat,
                        GL_UNSIGNED_BYTE,
                        image.pixels.data());

    flag = true;
  }

  if (desc.generate_mipmaps)
    glGenerateTextureMipmap(texture);

  auto fixed_filter =
    fix_filter(desc.generate_mipmaps, desc.min_filter, desc.mag_filter);
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, fixed_filter.first);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, fixed_filter.second);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, desc.wrap_s);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, desc.wrap_t);

  return RepoBase::create(desc.texture_name, texture, desc.texture_name);
}

std::shared_ptr<Texture>
TextureRepo::load_cubemap(const TextureDescriptor& desc)
{
  std::unordered_map<int, std::string> names = {
    {0, "_right.png"},
    {1, "_left.png"},
    {2, "_top.png"},
    {3, "_bottom.png"},
    {4, "_front.png"},
    {5, "_back.png"},
  };

  PNGImage image = PNGImage(desc.texture_name + names[0]);
  if (!image.is_valid()) {
    std::cerr << "ERROR: Invalid file" << desc.texture_name + names[0] << std::endl;
    return 0;
  }

  const auto width = image.width;
  const auto height = image.height;

  GLuint texture;
  glCreateTextures(desc.target, 1, &texture);

  int levels =
    desc.generate_mipmaps ? calculate_mip_levels(image.width, image.height) : 1;

  glTextureStorage2D(
    texture, levels, desc.internal_format, image.width, image.height);

  bool flag = false;

  for (int i = 0; i < 6; ++i) {
    std::string file_name = desc.texture_name + names[i];
    if (flag)
      image = PNGImage(file_name);

    if (!image.is_valid()) {
      std::cerr << "ERROR: Invalid file" << file_name << "\n";
      continue;
    }

    if (image.width != width || image.height != height) {
      std::cerr << "ERROR: Texture2DArray layer " << i
                << " has mismatched dimensions." << std::endl;
      continue;
    }

    GLenum sourceFormat = (image.channels == 4) ? GL_RGBA : GL_RGB;
    glTextureSubImage3D(texture,
                        0,
                        0,
                        0,
                        i,
                        width,
                        height,
                        1,
                        sourceFormat,
                        GL_UNSIGNED_BYTE,
                        image.pixels.data());

    flag = true;
  }

  if (desc.generate_mipmaps)
    glGenerateTextureMipmap(texture);

  auto fixed_filter =
    fix_filter(desc.generate_mipmaps, desc.min_filter, desc.mag_filter);
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, fixed_filter.first);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, fixed_filter.second);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, desc.wrap_s);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, desc.wrap_t);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_R, desc.wrap_r);

  return RepoBase::create(desc.texture_name, texture, desc.texture_name);
}
