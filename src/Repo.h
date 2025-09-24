#pragma once
#include "opengl.h"
#include <memory>
#include <string>
#include <unordered_map>

#include "Material.h"
#include "Mesh.h"
#include "Shader.h"

#define REPO_CLASS(ClassName, ResourceType)                                    \
  class ClassName {                                                            \
  private:                                                                     \
    std::unordered_map<unsigned int, std::shared_ptr<ResourceType>> resources; \
                                                                               \
  public:                                                                      \
    std::shared_ptr<ResourceType> create() {                                   \
      auto resource = std::make_shared<ResourceType>();                        \
      resources[resource->get_id()] = resource;                                \
      return resource;                                                         \
    }                                                                          \
    std::shared_ptr<ResourceType> get(unsigned int id) {                       \
      auto it = resources.find(id);                                            \
      if (it != resources.end()) {                                             \
        return it->second;                                                     \
      }                                                                        \
      return nullptr;                                                          \
    }                                                                          \
    void remove(unsigned int id) { resources.erase(id); }                      \
  };

class ShaderRepo {
private:
  std::unordered_map<std::string, GLuint> m_shaders;
  GLuint load_and_store_shader(const std::string &path);

public:
  GLuint get_shader(const std::string &path);
  void remove_shader(const std::string& path);
  ~ShaderRepo();
};

class TextureRepo {
private:
  std::unordered_map<std::string, GLuint> m_textures;
  GLuint load_and_store_texture(const std::string &names,
                                const TextureDescriptor &desc);
  GLuint load_and_store_tex_2d(const std::string &names,
                               const TextureDescriptor &desc);
  GLuint load_and_store_tex_2d_array(const std::string &names,
                                     const TextureDescriptor &desc);
  GLuint load_and_store_cubemap(const std::string &names,
                                const TextureDescriptor &desc);

public:
  GLuint
  get_texture(const std::string &name,
              const TextureDescriptor &desc = DEFAULT_TEXTURE_DESCRIPTOR);

  void remove_texture(const std::string &name);
  ~TextureRepo();
};

REPO_CLASS(MeshRepo, Mesh)
REPO_CLASS(ShaderProgramRepo, ShaderProgram)
REPO_CLASS(MaterialRepo, Material)

#undef REPO_CLASS
