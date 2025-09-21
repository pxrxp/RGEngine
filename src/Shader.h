#pragma once
#include "opengl.h"
#include <glm/glm.hpp>
#include <initializer_list>
#include <string>

class ShaderProgram {
private:
  GLuint m_id = 0;

public:
  ~ShaderProgram();

  GLuint get_id() const;
  void load(std::initializer_list<std::string> shaders);
  void bind();
  void unbind();

  void set_uniform(const char *name, GLfloat value);
  void set_uniform(const char *name, GLint value);
  void set_uniform(const char *name, const glm::vec2 &v);
  void set_uniform(const char *name, const glm::vec3 &v);
  void set_uniform(const char *name, const glm::vec4 &v);
  void set_uniform(const char *name, const glm::mat3 &m);
  void set_uniform(const char *name, const glm::mat4 &m);
};
