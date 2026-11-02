#pragma once
#include <engine/util/definitions.h>

class Shader
{
private:
  GLuint m_id = 0;

public:
  ~Shader();
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader& operator=(Shader&& other) noexcept;

  explicit Shader(GLuint id)
    : m_id(id)
  {
  }

  Shader(Shader&& other) noexcept
    : m_id(other.m_id)
  {
    other.m_id = 0;
  }

  GLuint get_id() const { return m_id; }
};
