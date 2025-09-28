#pragma once
#include "util/definitions.h"
#include <memory>

class Shader {
private:
  GLuint m_id = 0;

public:
  Shader(GLuint id);
  ~Shader();
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader(Shader&& other) noexcept;
  Shader& operator=(Shader&& other) noexcept;
};

using ShaderResourcePtr = std::shared_ptr<Shader>;
