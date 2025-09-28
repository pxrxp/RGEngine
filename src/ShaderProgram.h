#pragma once
#include "util/definitions.h"
#include <glm/glm.hpp>
#include <initializer_list>
#include <string>

class ShaderRepo;
class ShaderProgram
{
private:
  GLuint m_id = 0;

public:
  ShaderProgram(GLuint id)
    : m_id(id)
  {
  }

  GLuint get_id() const;
  void load(ShaderRepo& shader_repo,
            std::initializer_list<std::string> shaders);
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
