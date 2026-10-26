#pragma once
#include "util/definitions.h"
#include <cstdint>
#include <memory>
#include <string>

class Texture
{
private:
  GLuint m_id = 0;
  std::string m_name;
  std::uint32_t m_width = 0;
  std::uint32_t m_height = 0;

public:
  Texture(GLuint id, const std::string& name)
    : m_id(id)
    , m_name(name)
  {
  }
  ~Texture();

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture(Texture&& other) noexcept;
  Texture& operator=(Texture&& other) noexcept;

  GLuint get_id() const { return m_id; }
  std::string get_name() const { return m_name; }
};

using TextureStrongPtr = std::shared_ptr<Texture>;
