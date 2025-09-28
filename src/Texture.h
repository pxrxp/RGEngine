#pragma once
#include "util/definitions.h"
#include <memory>

class Texture {
private:
  GLuint m_id = 0;

public:
  Texture(GLuint id);
  ~Texture();
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture(Texture&& other) noexcept;
  Texture& operator=(Texture&& other) noexcept;
};

using TextureResourcePtr = std::shared_ptr<Texture>;
