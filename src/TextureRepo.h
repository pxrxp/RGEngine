#pragma once
#include "Repo.h"

class TextureRepo : RepoBase<std::string, Texture>
{
private:
  std::shared_ptr<Texture> load_texture(const std::string& names,
                                const TextureDescriptor& desc);
  std::shared_ptr<Texture> load_tex_2d(const std::string& names,
                               const TextureDescriptor& desc);
  std::shared_ptr<Texture> load_tex_2d_array(const std::string& names,
                                     const TextureDescriptor& desc);
  std::shared_ptr<Texture> load_cubemap(const std::string& names,
                                const TextureDescriptor& desc);

public:
  std::shared_ptr<Texture> get_texture(
    const std::string& name,
    const TextureDescriptor& desc = DEFAULT_TEXTURE_DESCRIPTOR);
};
