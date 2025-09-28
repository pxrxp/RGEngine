#pragma once

#include "Repo.h"
#include "ShaderRepo.h"
#include <memory>

using ShaderProgramStrongPtr = std::shared_ptr<ShaderProgram>;
using ShaderProgramWeakPtr = std::weak_ptr<ShaderProgram>;

class ShaderProgramRepo : public RepoBase<std::string, ShaderProgram>
{
private:
  ShaderRepo shader_repo;

public:
  ShaderProgramStrongPtr load_program(
    const std::string& name,
    std::initializer_list<std::string> shaders);
};
