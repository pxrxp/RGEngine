#pragma once
#include <memory>
#include <vector>

class Texture;
class ShaderProgram;

class Material {
private:
  static int id;
  std::vector<std::shared_ptr<Texture>> textures;
public:
  int get_id();
  void bind(std::shared_ptr<ShaderProgram>);
  void bind() {}
  void unbind() {}
};

inline int Material::id = 0;
