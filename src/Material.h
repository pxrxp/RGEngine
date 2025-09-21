#pragma once
#include <memory>
#include <vector>

class Texture;
class ShaderProgram;

class Material {
private:
  static int m_id;
  std::vector<std::shared_ptr<Texture>> m_textures;
public:
  int get_id();
  void bind(std::shared_ptr<ShaderProgram>);
  void bind() {}
  void unbind() {}
};

inline int Material::m_id = 0;
