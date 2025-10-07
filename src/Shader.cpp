#include "Shader.h"
#include "util/opengl.h"
#include <iostream>

Shader::~Shader()
{
  if (m_id) {
#ifndef NDEBUG
    std::cout << "Deleting shader " << m_id << std::endl;
#endif

    glDeleteShader(m_id);
  }
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
    m_name = std::move(other.m_name);
    other.m_id = 0;
  }
  return *this;
}
