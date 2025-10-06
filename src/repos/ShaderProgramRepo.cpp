#include "ShaderProgramRepo.h"
#include "../Shader.h"
#include "../ShaderProgram.h"
#include "../util/opengl.h"
#include <iostream>

ShaderProgramStrongPtr
ShaderProgramRepo::load_program(const ShaderProgramDescriptor& desc)
{
  if (auto program = RepoBase::get(desc.program_name); program) {
    if (!desc.is_getter_desc())
      std::cout << "Shader program " << desc.program_name << " already exists!"
                << std::endl;
    return program.value();
  }

  GLuint id = glCreateProgram();

  for (const auto& path : desc.shaders) {
    if (auto shader = shader_repo.load_shader(path); shader)
      glAttachShader(id, shader->get_id());
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
