#include "ShaderProgramRepo.h"
#include "Shader.h"
#include <iostream>

ShaderProgramStrongPtr
ShaderProgramRepo::load_program(const std::string& name, std::initializer_list<std::string> shaders)
{
  GLuint id = glCreateProgram();

  for (const auto& path : shaders) {
    glAttachShader(id, shader_repo.get_shader(path)->get_id());
  }
  glLinkProgram(id);

  GLint success;
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    char info[512];
    glGetProgramInfoLog(id, 512, nullptr, info);
    std::cerr << "Program link error:\n" << info << std::endl;
  }

  return RepoBase::create(name, id);
}
