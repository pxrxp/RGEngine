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
  Texture(GLuint id,
          const std::string& name,
          uint32_t width = 0,
          uint32_t height = 0)
    : m_id(id)
    , m_name(name)
    , m_width(width)
    , m_height(height)
  {
  }
  ~Texture();

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture(Texture&& other) noexcept;
  Texture& operator=(Texture&& other) noexcept;

  GLuint get_id() const { return m_id; }
  const std::string& get_name() const { return m_name; }
  std::uint32_t get_width() const { return m_width; }
  std::uint32_t get_height() const { return m_height; }

  float get_aspect_ratio() const
  {
    return (m_width > 0)
             ? (static_cast<float>(m_height) / static_cast<float>(m_width))
             : 1.0f;
  }
};

using TextureStrongPtr = std::shared_ptr<Texture>;
