#include "Texture.h"
#include "util/opengl.h"
#include <glad/glad.h>
#include <iostream>

Texture::~Texture()
{
  if (m_id) {
#ifndef NDEBUG
    std::cout << "Deleting textures " << m_id << std::endl;
#endif

    glDeleteTextures(1, &m_id);
  }
}

Texture::Texture(Texture&& other) noexcept
  : m_id(other.m_id)
{
  other.m_id = 0;
}

Texture&
Texture::operator=(Texture&& other) noexcept
{
  if (this != &other) {
    if (m_id != 0) {
#ifndef NDEBUG
      std::cout << "Deleting textures " << m_id << std::endl;
#endif
      glDeleteTextures(1, &m_id);
    }
    m_id = other.m_id;
    other.m_id = 0;
  }
  return *this;
}
