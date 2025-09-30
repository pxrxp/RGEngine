#pragma once

#include "Repo.h"
#include "ShaderRepo.h"
#include <memory>
#include <vector>

class ShaderProgram;
using ShaderProgramStrongPtr = std::shared_ptr<ShaderProgram>;
using ShaderProgramWeakPtr = std::weak_ptr<ShaderProgram>;

struct ShaderProgramDescriptor
{
  std::string program_name;
  std::vector<std::string> shaders;

  auto operator<=>(const ShaderProgramDescriptor&) const = default;
};

class ShaderProgramRepo : public RepoBase<std::string, ShaderProgram>
{
public:
  ShaderRepo shader_repo;
  ShaderProgramStrongPtr load_program(const ShaderProgramDescriptor& shaders);
};
