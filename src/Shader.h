#pragma once
#include "util/definitions.h"
#include <memory>

class Shader
{
private:
  std::string m_name;
  GLuint m_id = 0;

public:
  ~Shader();
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader& operator=(Shader&& other) noexcept;

  Shader(const std::string& shader_name, GLuint id)
    : m_name(std::move(shader_name))
    , m_id(id)
  {
  }

  Shader(Shader&& other) noexcept
    : m_name(std::move(other.m_name))
    , m_id(other.m_id)
  {
    other.m_id = 0;
  }

  GLuint get_id() const { return m_id; }
};

using ShaderResourcePtr = std::shared_ptr<Shader>;
