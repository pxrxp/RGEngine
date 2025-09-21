#pragma once
#include "opengl.h"
#include <array>
#include <string>

class Texture {
private:
  GLuint id = 0;
  GLint texture_unit = -1;

public:
  void load(std::array<std::string, 6> relative_path, bool generate_mipmap,
            GLenum min_filter, GLenum max_filter);
  void load(std::string relative_path, GLenum target = GL_TEXTURE_2D,
            bool generate_mipmap = true, GLenum min_filter = GL_LINEAR,
            GLenum max_filter = GL_LINEAR);
  GLuint get_id() const;
  void bind();
  void unbind();

  GLint get_texture_unit() const;
};

