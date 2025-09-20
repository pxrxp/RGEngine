#pragma once
#include "opengl.h"
#include <glm/glm.hpp>
#include <initializer_list>
#include <string>
#include <unordered_map>

class ShaderProgram {
private:
  GLuint id = 0;

public:
  ~ShaderProgram();

  GLuint get_id() const;
  void load_shaders(std::initializer_list<std::string> paths);
  void bind();
  void unbind();

  void set_uniform(const char *name, GLfloat value,
                   bool bind_program = false) const;
  void set_uniform(const char *name, GLint value,
                   bool bind_program = false) const;
  void set_uniform(const char *name, const glm::vec2 &v,
                   bool bind_program = false) const;
  void set_uniform(const char *name, const glm::vec3 &v,
                   bool bind_program = false) const;
  void set_uniform(const char *name, const glm::vec4 &v,
                   bool bind_program = false) const;
  void set_uniform(const char *name, const glm::mat3 &m,
                   bool bind_program = false) const;
  void set_uniform(const char *name, const glm::mat4 &m,
                   bool bind_program = false) const;
  void set_texture_unit(const char *name, GLint unit,
                        bool bind_program = false) const;
};

class ShaderRepo {
private:
  std::unordered_map<std::string, GLuint> shaders;
  GLuint load_and_store_shader(std::string path);

public:
  GLuint get_shader(std::string path);
};
