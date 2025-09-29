#include "ShaderProgramRepo.h"
#include "ShaderProgram.h"
#include "util/opengl.h"
#include "Shader.h"
#include <iostream>

ShaderProgramStrongPtr
ShaderProgramRepo::load_program(const ShaderProgramDescriptor& desc)
{
  if (auto program = RepoBase::get(desc.program_name); program) {
    std::cerr << "Shader program " << desc.program_name << " already exists!" << std::endl;
    return program.value();
  }
  
  GLuint id = glCreateProgram();

  for (const auto& path : desc.shaders) {
    if (auto shader = shader_repo.get(path); shader.has_value())
      glAttachShader(id, shader.value()->get_id());
  }
  glLinkProgram(id);

  GLint success;
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    char info[512];
    glGetProgramInfoLog(id, 512, nullptr, info);
    std::cerr << "Program link error:\n" << info << std::endl;
  }

  return RepoBase::create(desc.program_name, id);
}
