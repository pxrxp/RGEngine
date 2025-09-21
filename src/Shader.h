#pragma once
#include "opengl.h"
#include <glm/glm.hpp>
#include <initializer_list>
#include <string>

class ShaderProgram {
private:
  GLuint m_id = 0;

public:
  static GLuint previous_program;
  static GLuint current_program;
  ~ShaderProgram();

  GLuint get_id() const;
  void load_shaders(std::initializer_list<std::string> paths);
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

inline GLuint ShaderProgram::current_program = 0;
inline GLuint ShaderProgram::previous_program = 0;
