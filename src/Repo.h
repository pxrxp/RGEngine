#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "Material.h"
#include "Mesh.h"
#include "Shader.h"

template<typename Key, typename Resource>
class RepoBase
{
protected:
  std::unordered_map<Key, std::shared_ptr<Resource>> m_resources;

  template<typename... Args>
  std::shared_ptr<Resource> create(Args&&... args)
  {
    auto resource = std::make_shared<Resource>(std::forward<Args>(args)...);
    m_resources[resource->get_id()] = resource;
    return resource;
  }

  std::shared_ptr<Resource> get(const Key& id)
  {
    auto it = m_resources.find(id);
    if (it != m_resources.end()) {
      return it->second;
    }
    return nullptr;
  }

  void remove(const Key& id) { m_resources.erase(id); }

public:
  virtual ~RepoBase() = default;
  void clear() { m_resources.clear(); }
};

class ShaderRepo : RepoBase<std::string, Shader>
{
private:
  GLuint load_and_store_shader(const std::string& path);

public:
  GLuint get_shader(const std::string& path);
  void remove_shader(const std::string& path);
  void clear();
};

class TextureRepo : RepoBase<std::string, Texture>
{
private:
  GLuint load_and_store_texture(const std::string& names,
                                const TextureDescriptor& desc);
  GLuint load_and_store_tex_2d(const std::string& names,
                               const TextureDescriptor& desc);
  GLuint load_and_store_tex_2d_array(const std::string& names,
                                     const TextureDescriptor& desc);
  GLuint load_and_store_cubemap(const std::string& names,
                                const TextureDescriptor& desc);

public:
  GLuint get_texture(
    const std::string& name,
    const TextureDescriptor& desc = DEFAULT_TEXTURE_DESCRIPTOR);

  void remove_texture(const std::string& name);
  void clear();
};

class MeshRepo : public RepoBase<Mesh>
{};
class ShaderProgramRepo : public RepoBase<ShaderProgram>
{};
class MaterialRepo : public RepoBase<Material>
{};
