#include "Repo.h"
#include "util/png_image.h"
#include <algorithm>

inline int
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

GLuint
TextureRepo::load_and_store_texture(const std::string& name,
                                    const TextureDescriptor& desc)
{

  if (name.empty()) {
    std::cerr << "Provide at least one texture!\n";
    return 0;
  }

  GLuint id = 0;

  switch (desc.target) {
    case GL_TEXTURE_2D:
      id = load_and_store_tex_2d(name, desc);
      break;

    case GL_TEXTURE_CUBE_MAP:
      id = load_and_store_cubemap(name, desc);
      break;

    case GL_TEXTURE_2D_ARRAY:
      id = load_and_store_tex_2d_array(name, desc);
      break;

    default:
      std::cerr
        << "Invalid target! Expected GL_TEXTURE_2D | GL_TEXTURE_CUBE_MAP "
           "| GL_TEXTURE_2D_ARRAY\n";
      break;
  }

  if (id != 0)
    m_textures[name] = id;

  return id;
}

GLuint
TextureRepo::load_and_store_tex_2d(const std::string& name,
                                   const TextureDescriptor& desc)
{

  PNGImage image(name + ".png");
  if (!image.is_valid()) {
    std::cerr << "ERROR: Invalid file" << name + ".png\n";
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

  return texture;
}

GLuint
TextureRepo::load_and_store_tex_2d_array(const std::string& name,
                                         const TextureDescriptor& desc)
{
  PNGImage image = PNGImage(name + "_0.png");
  if (!image.is_valid()) {
    std::cerr << "ERROR: Invalid file" << name + "_0.png\n";
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
    std::string file_name = name + "_" + std::to_string(i) + ".png";
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

  return texture;
}

GLuint
TextureRepo::load_and_store_cubemap(const std::string& name,
                                    const TextureDescriptor& desc)
{
  PNGImage image = PNGImage(name + "_0.png");
  if (!image.is_valid()) {
    std::cerr << "ERROR: Invalid file" << name + "_0.png\n";
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
    std::string file_name = name + "_" + std::to_string(i) + ".png";
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

  return texture;
}

GLuint
TextureRepo::get_texture(const std::string& name, const TextureDescriptor& desc)
{
  bool not_found = m_textures.find(name) == m_textures.end();
  if (not_found) {
    return load_and_store_texture(name, desc);
  } else {
    return m_textures[name];
  }
}

void
TextureRepo::remove_texture(const std::string& name)
{
  auto it = m_textures.find(name);
  if (it != m_textures.end()) {
    glDeleteTextures(1, &it->second);
    m_textures.erase(it);
  }
}

void TextureRepo::clear()
{
  for (auto& [name, id] : m_textures) {
    glDeleteTextures(1, &id);
  }

  m_textures.clear();
}
