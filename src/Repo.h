#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "Material.h"
#include "Mesh.h"
#include "Shader.h"

template <typename ResourceType>
class RepoBase
{
protected:
  std::unordered_map<unsigned int, std::shared_ptr<ResourceType>> m_resources;

public:
  virtual ~RepoBase() = default;

  template <typename... Args>
  std::shared_ptr<ResourceType> create(Args&&... args)
  {
    auto resource = std::make_shared<ResourceType>(std::forward<Args>(args)...);
    m_resources[resource->get_id()] = resource;
    return resource;
  }

  std::shared_ptr<ResourceType> get(unsigned int id)
  {
    auto it = m_resources.find(id);
    if (it != m_resources.end()) {
      return it->second;
    }
    return nullptr;
  }

  void remove(unsigned int id)
  {
    m_resources.erase(id);
  }
};

class ShaderRepo
{
private:
  std::unordered_map<std::string, GLuint> m_shaders;
  GLuint load_and_store_shader(const std::string& path);

public:
  GLuint get_shader(const std::string& path);
  void remove_shader(const std::string& path);
  ~ShaderRepo();
};

class TextureRepo
{
private:
  std::unordered_map<std::string, GLuint> m_textures;
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
  ~TextureRepo();
};

class MeshRepo : public RepoBase<Mesh> {
  public:
    ~MeshRepo();
};
class ShaderProgramRepo : public RepoBase<ShaderProgram> {
  public:
    ~ShaderProgramRepo();
};
class MaterialRepo : public RepoBase<Material> {
  public:
    ~MaterialRepo() = default;
};
