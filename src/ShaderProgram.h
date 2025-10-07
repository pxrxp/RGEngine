#pragma once
#include "util/definitions.h"
#include <glm/glm.hpp>
#include <string>

class ShaderRepo;
class ShaderProgram
{
private:
  std::string m_name;
  GLuint m_id = 0;

public:
  ShaderProgram(const std::string& program_name, GLuint id)
    : m_name(std::move(program_name))
    , m_id(id)
  {
  }

  ShaderProgram(const ShaderProgram&) = delete;
  ShaderProgram& operator=(const ShaderProgram&) = delete;
  ShaderProgram& operator=(ShaderProgram&& other) noexcept;

  ShaderProgram(ShaderProgram&& other) noexcept
    : m_name(std::move(other.m_name))
    , m_id(other.m_id)
  {
    other.m_id = 0;
  }

  GLuint get_id() const { return m_id; }
  void bind();
  void unbind();

  ShaderProgram& set_uniform(const char* name, GLfloat value);
  ShaderProgram& set_uniform(const char* name, GLint value);
  ShaderProgram& set_uniform(const char* name, const glm::vec2& v);
  ShaderProgram& set_uniform(const char* name, const glm::vec3& v);
  ShaderProgram& set_uniform(const char* name, const glm::vec4& v);
  ShaderProgram& set_uniform(const char* name, const glm::mat3& m);
  ShaderProgram& set_uniform(const char* name, const glm::mat4& m);

  ~ShaderProgram();
};
