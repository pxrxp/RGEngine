#include "Shader.h"
#include "Repo.h"
#include "opengl.h"
#include "util.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <initializer_list>
#include <string>
#include <iostream>

ShaderRepo repo;

GLuint ShaderProgram::get_id() const {
  return m_id;
}

void ShaderProgram::bind() {
  glUseProgram(m_id);
}

void ShaderProgram::unbind() {
  glUseProgram(0);
}

ShaderProgram::~ShaderProgram() {
  glDeleteProgram(m_id);
}

void ShaderProgram::load(std::initializer_list<std::string> shaders) {
  if (m_id == 0) {
    m_id = glCreateProgram();
  }

  for (const auto& path: shaders) {
    GLuint shader = repo.get_shader(path);
    glAttachShader(m_id, shader);
  }
  glLinkProgram(m_id);

  GLint success;
  glGetProgramiv(m_id, GL_LINK_STATUS, &success);
  if (!success) {
    char info[512];
    glGetProgramInfoLog(m_id, 512, nullptr, info);
    std::cerr << "Program link error:\n" << info << "\n";
  }

}

void ShaderProgram::set_uniform(const char* name, float value) {
    GLint loc = glGetUniformLocation(m_id, name);
    if (loc != -1) glProgramUniform1f(m_id, loc, value);
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << m_id << "\n";
}

void ShaderProgram::set_uniform(const char* name, int value) {
    GLint loc = glGetUniformLocation(m_id, name);
    if (loc != -1) glProgramUniform1i(m_id, loc, value);
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << m_id << "\n";
}

void ShaderProgram::set_uniform(const char* name, const glm::vec2& v) {
    GLint loc = glGetUniformLocation(m_id, name);
    if (loc != -1) glProgramUniform2fv(m_id, loc, 1, glm::value_ptr(v));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << m_id << "\n";
}

void ShaderProgram::set_uniform(const char* name, const glm::vec3& v) {
    GLint loc = glGetUniformLocation(m_id, name);
    if (loc != -1) glProgramUniform3fv(m_id, loc, 1, glm::value_ptr(v));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << m_id << "\n";
}

void ShaderProgram::set_uniform(const char* name, const glm::vec4& v) {
    GLint loc = glGetUniformLocation(m_id, name);
    if (loc != -1) glProgramUniform4fv(m_id, loc, 1, glm::value_ptr(v));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << m_id << "\n";
}

void ShaderProgram::set_uniform(const char* name, const glm::mat3& m) {
    GLint loc = glGetUniformLocation(m_id, name);
    if (loc != -1) glProgramUniformMatrix3fv(m_id, loc, 1, GL_FALSE, glm::value_ptr(m));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << m_id << "\n";
}

void ShaderProgram::set_uniform(const char* name, const glm::mat4& m) {
    GLint loc = glGetUniformLocation(m_id, name);
    if (loc != -1) glProgramUniformMatrix4fv(m_id, loc, 1, GL_FALSE, glm::value_ptr(m));
    else std::cerr << "Warning: uniform \"" << name << "\" not found in program " << m_id << "\n";
}
