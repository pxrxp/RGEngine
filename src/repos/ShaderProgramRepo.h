#pragma once

#include "RepoBase.h"
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

  bool is_getter_desc() const { return shaders.empty(); }
};

class ShaderProgramRepo : public RepoBase<std::string, ShaderProgram>
{
public:
  ShaderRepo shader_repo;
  ShaderProgramStrongPtr load_program(const ShaderProgramDescriptor& shaders);
};
