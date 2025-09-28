#include "Shader.h"
#include "util/opengl.h"
#include <iostream>

Shader::Shader(GLuint id)
  : m_id(id)
{
}
Shader::~Shader()
{
  if (m_id) {
#ifndef NDEBUG
    std::cout << "Deleting shader " << m_id << std::endl;
#endif

    glDeleteShader(m_id);
  }
}

Shader::Shader(Shader&& other) noexcept
  : m_id(other.m_id)
{
  other.m_id = 0;
}

Shader&
Shader::operator=(Shader&& other) noexcept
{
  if (this != &other) {
    if (m_id != 0) {
#ifndef NDEBUG
      std::cout << "Deleting shader " << m_id << std::endl;
#endif
      glDeleteShader(m_id);
    }
    m_id = other.m_id;
    other.m_id = 0;
  }
  return *this;
}
