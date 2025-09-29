#pragma once

#include "Repo.h"
#include "ShaderRepo.h"
#include <memory>

class ShaderProgram;
using ShaderProgramStrongPtr = std::shared_ptr<ShaderProgram>;
using ShaderProgramWeakPtr = std::weak_ptr<ShaderProgram>;

struct ShaderProgramDescriptor
{
  std::string program_name;
  std::initializer_list<std::string> shaders;
};

class ShaderProgramRepo : public RepoBase<std::string, ShaderProgram>
{
public:
  ShaderRepo shader_repo;
  ShaderProgramStrongPtr load_program(const ShaderProgramDescriptor& shaders);
};
