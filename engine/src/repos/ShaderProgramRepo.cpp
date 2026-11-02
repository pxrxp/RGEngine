#include <engine/Shader.h>
#include <engine/ShaderProgram.h>
#include <engine/repos/ShaderProgramRepo.h>
#include <engine/util/log.h>
#include <engine/util/opengl.h>
#include <iostream>

ShaderProgramStrongPtr
ShaderProgramRepo::load_program(const ShaderProgramDescriptor& desc)
{
  if (auto program = RepoBase::get(desc.program_name); program)
    return program.value();

  const GLuint id = glCreateProgram();
  if (id == 0)
    return {};

  for (const auto& path : desc.shaders) {
    const auto shader = shader_repo.load_shader(path);
    if (!shader) {
      glDeleteProgram(id);
      return {};
    }
    glAttachShader(id, shader->get_id());
  }

  glLinkProgram(id);

  GLint success = GL_FALSE;
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (success != GL_TRUE) {
    char info[512] = {};
    glGetProgramInfoLog(id, sizeof(info), nullptr, info);
    Util::log_err() << "Program link error:\n" << info << std::endl;
    glDeleteProgram(id);
    return {};
  }

  return RepoBase::create(desc.program_name, id);
}
