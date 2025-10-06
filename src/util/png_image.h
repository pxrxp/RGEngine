#pragma once

#include <cstdint>
#include <string>
#include <vector>

class PNGImage
{
private:
  bool m_is_valid = false;

public:
  uint32_t width = 0;
  uint32_t height = 0;
  uint8_t channels = 0;
  std::string file_name;
  std::vector<unsigned char> pixels;

  explicit PNGImage(const std::string& relative_path);

  bool is_valid() const;
};
