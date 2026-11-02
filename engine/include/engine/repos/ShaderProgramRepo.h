#pragma once

#include <engine/repos/RepoBase.h>
#include <engine/repos/ShaderRepo.h>
#include <memory>
#include <string>
#include <vector>

class ShaderProgram;
using ShaderProgramStrongPtr = std::shared_ptr<ShaderProgram>;
using ShaderProgramWeakPtr = std::weak_ptr<ShaderProgram>;

struct ShaderProgramDescriptor
{
  std::string program_name;
  std::vector<std::string> shaders;
};

class ShaderProgramRepo : public RepoBase<std::string, ShaderProgram>
{
public:
  ShaderRepo shader_repo;
  ShaderProgramStrongPtr load_program(const ShaderProgramDescriptor& shaders);
};
