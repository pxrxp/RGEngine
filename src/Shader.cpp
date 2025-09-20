#include "Shader.h"
#include "opengl.h"
#include "util.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <iostream>

std::unordered_map<char, int> shader_map =
{ {'t', GL_VERTEX_SHADER},         // .vert
  {'g', GL_FRAGMENT_SHADER},       // .frag
  {'o', GL_GEOMETRY_SHADER},       // .geo
  {'p', GL_COMPUTE_SHADER},        // .comp
  {'l', GL_TESS_CONTROL_SHADER},   // .ctrl
  {'a', GL_TESS_EVALUATION_SHADER} // .eva
};

ShaderRepo repo;

GLuint ShaderRepo::load_and_store_shader(std::string path) {
    std::string shader_src = read_file("shaders/" + path);
    const char *shader_src_cstr = shader_src.c_str();
    GLuint shader = glCreateShader(shader_map[path[path.size()-6]]);

    GLint success;
    glShaderSource(shader, 1, &shader_src_cstr, nullptr);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      char info[512];
      glGetShaderInfoLog(shader, 512, nullptr, info);
      std::cerr << "Error while compiling " << path << ":\n" << info << "\n";
    }

    this->shaders[path] = shader;
    return shader;
}

GLuint ShaderRepo::get_shader(std::string path) {
  bool not_found = shaders.find(path) == shaders.end();
  if (not_found) {
    return load_and_store_shader(path);
  } else {
    return shaders[path];
  }
}

GLuint ShaderProgram::get_id() const {
  return id;
}

void ShaderProgram::bind() {
  glUseProgram(this->id);
}

void ShaderProgram::unbind() {
  glUseProgram(0);
}

ShaderProgram::~ShaderProgram() {
  glDeleteProgram(this->id);
}

void ShaderProgram::load_shaders(std::initializer_list<std::string> paths) {
  if (this->id == 0) {
    this->id = glCreateProgram();
  }

  for (const auto& path: paths) {
    GLuint shader = repo.get_shader(path);
    glAttachShader(this->id, shader);
  }
  glLinkProgram(this->id);

  GLint success;
  glGetProgramiv(this->id, GL_LINK_STATUS, &success);
  if (!success) {
    char info[512];
    glGetProgramInfoLog(this->id, 512, nullptr, info);
    std::cerr << "Program link error:\n" << info << "\n";
  }
}

void ShaderProgram::set_uniform(const char* name, float value, bool bind_program) const {
    if (bind_program) glUseProgram(this->id);
    GLint loc = glGetUniformLocation(this->id, name);
    if (loc != -1) glUniform1f(loc, value);
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << this->id << "\n";
    if (bind_program) glUseProgram(0);
}

void ShaderProgram::set_uniform(const char* name, int value, bool bind_program) const {
    if (bind_program) glUseProgram(this->id);
    GLint loc = glGetUniformLocation(this->id, name);
    if (loc != -1) glUniform1i(loc, value);
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << this->id << "\n";
    if (bind_program) glUseProgram(0);
}

void ShaderProgram::set_uniform(const char* name, const glm::vec2& v, bool bind_program) const {
    if (bind_program) glUseProgram(this->id);
    GLint loc = glGetUniformLocation(this->id, name);
    if (loc != -1) glUniform2fv(loc, 1, glm::value_ptr(v));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << this->id << "\n";
    if (bind_program) glUseProgram(0);
}

void ShaderProgram::set_uniform(const char* name, const glm::vec3& v, bool bind_program) const {
    if (bind_program) glUseProgram(this->id);
    GLint loc = glGetUniformLocation(this->id, name);
    if (loc != -1) glUniform3fv(loc, 1, glm::value_ptr(v));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << this->id << "\n";
    if (bind_program) glUseProgram(0);
}

void ShaderProgram::set_uniform(const char* name, const glm::vec4& v, bool bind_program) const {
    if (bind_program) glUseProgram(this->id);
    GLint loc = glGetUniformLocation(this->id, name);
    if (loc != -1) glUniform4fv(loc, 1, glm::value_ptr(v));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << this->id << "\n";
    if (bind_program) glUseProgram(0);
}

void ShaderProgram::set_uniform(const char* name, const glm::mat3& m, bool bind_program) const {
    if (bind_program) glUseProgram(this->id);
    GLint loc = glGetUniformLocation(this->id, name);
    if (loc != -1) glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << this->id << "\n";
    if (bind_program) glUseProgram(0);
}

void ShaderProgram::set_uniform(const char* name, const glm::mat4& m, bool bind_program) const {
    if (bind_program) glUseProgram(this->id);
    GLint loc = glGetUniformLocation(this->id, name);
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << this->id << "\n";
    if (bind_program) glUseProgram(0);
}

void ShaderProgram::set_texture_unit(const char* name, GLint unit, bool bind_program) const {
    if (bind_program) glUseProgram(this->id);
    GLint loc = glGetUniformLocation(this->id, name);
    if (loc != -1) glUniform1i(loc, 0);
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << this->id << "\n";
    if (bind_program) glUseProgram(0);
}
