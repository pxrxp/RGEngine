#include "ShaderProgram.h"
#include "util/opengl.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

ShaderProgram::~ShaderProgram() {
  if (m_id) { 
#ifndef NDEBUG
    std::cout << "Deleting shader program " << m_id << std::endl;
#endif

    glDeleteProgram(m_id);
  }
}

ShaderProgram&
ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
  if (this != &other) {
    if (m_id != 0) {
#ifndef NDEBUG
      std::cout << "Deleting shader program " << m_id << std::endl;
#endif
      glDeleteProgram(m_id);
    }
    m_id = other.m_id;
    m_name = std::move(other.m_name);
    other.m_id = 0;
  }
  return *this;
}

void
ShaderProgram::bind()
{
  glUseProgram(m_id);
}

void
ShaderProgram::unbind()
{
  glUseProgram(0);
}

ShaderProgram&
ShaderProgram::set_uniform(const char* name, float value)
{
  GLint loc = glGetUniformLocation(m_id, name);
  if (loc != -1)
    glProgramUniform1f(m_id, loc, value);
  else
    std::cerr << "Warning: uniform \"" << name << "\" not found in program "
              << m_id << "\n";
  return *this;
}

ShaderProgram&
ShaderProgram::set_uniform(const char* name, int value)
{
  GLint loc = glGetUniformLocation(m_id, name);
  if (loc != -1)
    glProgramUniform1i(m_id, loc, value);
  else
    std::cerr << "Warning: uniform \"" << name << "\" not found in program "
              << m_id << "\n";
  return *this;
}

ShaderProgram&
ShaderProgram::set_uniform(const char* name, const glm::vec2& v)
{
  GLint loc = glGetUniformLocation(m_id, name);
  if (loc != -1)
    glProgramUniform2fv(m_id, loc, 1, glm::value_ptr(v));
  else
    std::cerr << "Warning: uniform \"" << name << "\" not found in program "
              << m_id << "\n";
  return *this;
}

ShaderProgram&
ShaderProgram::set_uniform(const char* name, const glm::vec3& v)
{
  GLint loc = glGetUniformLocation(m_id, name);
  if (loc != -1)
    glProgramUniform3fv(m_id, loc, 1, glm::value_ptr(v));
  else
    std::cerr << "Warning: uniform \"" << name << "\" not found in program "
              << m_id << "\n";
  return *this;
}

ShaderProgram&
ShaderProgram::set_uniform(const char* name, const glm::vec4& v)
{
  GLint loc = glGetUniformLocation(m_id, name);
  if (loc != -1)
    glProgramUniform4fv(m_id, loc, 1, glm::value_ptr(v));
  else
    std::cerr << "Warning: uniform \"" << name << "\" not found in program "
              << m_id << "\n";
  return *this;
}

ShaderProgram&
ShaderProgram::set_uniform(const char* name, const glm::mat3& m)
{
  GLint loc = glGetUniformLocation(m_id, name);
  if (loc != -1)
    glProgramUniformMatrix3fv(m_id, loc, 1, GL_FALSE, glm::value_ptr(m));
  else
    std::cerr << "Warning: uniform \"" << name << "\" not found in program "
              << m_id << "\n";
  return *this;
}

ShaderProgram&
ShaderProgram::set_uniform(const char* name, const glm::mat4& m)
{
  GLint loc = glGetUniformLocation(m_id, name);
  if (loc != -1)
    glProgramUniformMatrix4fv(m_id, loc, 1, GL_FALSE, glm::value_ptr(m));
  else
    std::cerr << "Warning: uniform \"" << name << "\" not found in program "
              << m_id << "\n";
  return *this;
}
