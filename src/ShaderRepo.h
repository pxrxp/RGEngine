#pragma once

#include "Repo.h"
#include <memory>

class Shader;
using ShaderStrongPtr = std::shared_ptr<Shader>;
using ShaderWeakPtr = std::weak_ptr<Shader>;

class ShaderRepo : public RepoBase<std::string, Shader>
{
public:
  ShaderStrongPtr load_shader(const std::string& path);
};
