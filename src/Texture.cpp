#include "Texture.h"
#include "opengl.h"
#include "png_image.h"
#include <iostream>

GLint Texture::get_texture_unit() const { return m_texture_unit; }

GLuint Texture::get_id() const { return m_id; }

void Texture::load(std::array<std::string, 6> cube_faces_relative_path,
                   bool generate_mipmap, GLenum min_filter, GLenum max_filter) {

  glGenTextures(1, &m_id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

  for (unsigned int i = 0; i < 6; i++) {
    PNGImage image(cube_faces_relative_path[i]);
    GLenum format;
    if (image.channels == 3) {
      format = GL_RGB;
    } else if (image.channels == 4) {
      format = GL_RGBA;
    } else {
      std::cerr << "Unsupported image (use RGB / RGBA): "
                << cube_faces_relative_path[i] << std::endl;
      return;
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, image.width,
                 image.height, 0, format, GL_UNSIGNED_BYTE,
                 image.pixels.data());
  }

  GLenum _min_filter, _max_filter;

  if (generate_mipmap) {
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    if (min_filter == GL_LINEAR)
      _min_filter = GL_LINEAR_MIPMAP_LINEAR;

    if (min_filter == GL_NEAREST)
      _min_filter = GL_NEAREST_MIPMAP_NEAREST;
  }

  if (max_filter == GL_NEAREST_MIPMAP_NEAREST ||
      max_filter == GL_NEAREST_MIPMAP_LINEAR)
    _max_filter = GL_NEAREST;

  if (max_filter == GL_LINEAR_MIPMAP_LINEAR ||
      max_filter == GL_LINEAR_MIPMAP_NEAREST)
    _max_filter = GL_LINEAR;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _max_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Texture::load(std::string relative_path, GLenum target,
                   bool generate_mipmap, GLenum min_filter, GLenum max_filter) {

  if (target >= GL_TEXTURE0 && target <= GL_TEXTURE31) {
    m_texture_unit = target - GL_TEXTURE0;
  }

  PNGImage image(relative_path);
  GLenum format;
  if (image.channels == 3) {
    format = GL_RGB;
  } else if (image.channels == 4) {
    format = GL_RGBA;
  } else {
    std::cerr << "Unsupported image (use RGB / RGBA): " << relative_path
              << std::endl;
    return;
  }

  glGenTextures(1, &m_id);
  glBindTexture(target, m_id);
  glTexImage2D(target, 0, format, image.width, image.height, 0, format,
               GL_UNSIGNED_BYTE, image.pixels.data());

  GLenum _min_filter, _max_filter;

  if (generate_mipmap) {
    glGenerateMipmap(target);

    if (min_filter == GL_LINEAR)
      _min_filter = GL_LINEAR_MIPMAP_LINEAR;

    if (min_filter == GL_NEAREST)
      _min_filter = GL_NEAREST_MIPMAP_NEAREST;
  }

  if (max_filter == GL_NEAREST_MIPMAP_NEAREST ||
      max_filter == GL_NEAREST_MIPMAP_LINEAR)
    _max_filter = GL_NEAREST;

  if (max_filter == GL_LINEAR_MIPMAP_LINEAR ||
      max_filter == GL_LINEAR_MIPMAP_NEAREST)
    _max_filter = GL_LINEAR;

  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, _min_filter);
  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, _max_filter);
  glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
