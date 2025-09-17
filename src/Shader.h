#pragma once
#include <string>
#include <unordered_map>
#include <initializer_list>
#include <glm/glm.hpp>

class ShaderProgram {
private:
  unsigned int id;

public:
  ShaderProgram();
  ~ShaderProgram();
 
  void load_shaders(std::initializer_list<std::string> paths);
  void bind();
  void unbind();

  void set_uniform(const char* name, float value, bool bind_program = false) const;
  void set_uniform(const char* name, int value, bool bind_program = false) const;
  void set_uniform(const char* name, const glm::vec2& v, bool bind_program = false) const;
  void set_uniform(const char* name, const glm::vec3& v, bool bind_program = false) const;
  void set_uniform(const char* name, const glm::vec4& v, bool bind_program = false) const;
  void set_uniform(const char* name, const glm::mat3& m, bool bind_program = false) const;
  void set_uniform(const char* name, const glm::mat4& m, bool bind_program = false) const;
};

class ShaderRepo {
private:
  std::unordered_map<std::string, unsigned int> shaders;
  unsigned int load_and_store_shader(std::string path);
public:
  unsigned int get_shader(std::string path);
};
