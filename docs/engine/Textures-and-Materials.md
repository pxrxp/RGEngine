# Textures and materials

## Textures

`TextureDescriptor` (`engine/include/engine/repos/TextureRepo.h`) describes a texture:

```cpp
TextureDescriptor{
  .texture_name = "concrete",       // cache key; also the file stem
  .target = GL_TEXTURE_2D,          // GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP
  .internal_format = GL_RGBA8,
  .wrap_s = GL_REPEAT, .wrap_t = GL_REPEAT,
  .min_filter = GL_LINEAR_MIPMAP_LINEAR, .mag_filter = GL_LINEAR,
  .generate_mipmaps = true,
  .max_mip_level = -1
};
```

`TextureRepo::load_texture()` loads PNG files from `resources/`:

- `GL_TEXTURE_2D` reads `<texture_name>.png`;
- `GL_TEXTURE_CUBE_MAP` reads `<texture_name>_right.png`, `_left`, `_top`, `_bottom`, `_front`, `_back` (all faces must match the first face's size).

Mip level count is computed from the image size when mipmaps are requested; the filter helper downgrades mipmap filters when they are not. Cubemap faces are uploaded with `glTexSubImage2D` after `glTextureStorage2D`. Failures log and return `nullptr`, so a material with a missing texture lacks that texture rather than crashing.

## Materials

A `MaterialDescriptor` names a material, a shader program descriptor, a list of texture descriptors, and a map of uniforms:

```cpp
MaterialDescriptor{
  .material_name = "m4_colt_material",
  .shader_program_desc = { .program_name = "cube_program",
                           .shaders = { "cube.vert", "cube.frag" } },
  .texture_desc_list = { { .texture_name = "m4_colt" } },
  .uniforms = { { "u_diffuse", TextureName("m4_colt") },
                { "u_use_texture", 1 },
                { "u_is_gun", 1 } } };
```

`MaterialRepo::load_material()` loads the textures through its own `tex_repo`, loads/links the program through `shader_program_repo`, and caches by material name. Loading an existing name returns the cached material, textures included.

`UniformValue` is a variant of `int`, `float`, `glm::vec2/3/4`, `glm::mat3/4`, and `TextureName`. A `TextureName` value is resolved at bind time: the material looks up a texture with that name, assigns it a texture unit through `MaterialRepo`, binds it with `glBindTextureUnit`, and uploads the unit as an `int` uniform.

## Bind and unbind

`Material::bind()` uploads all non-texture uniforms and resolves all texture uniforms. `Material::unbind()` releases only the slots the material acquired. Materials are bound by `Renderer` and released when the material changes and after the last packet.

## Slot accounting

`MaterialRepo` keeps `empty_texture_slots` and `full_texture_slots` derived from `GL_MAX_TEXTURE_IMAGE_UNITS`, plus a texture-name → slot map. `App::init()` calls `clear_all_slots()` after the GL context exists. A texture name keeps its unit until the material using it is unbound; if no unit is free, binding logs and skips the texture.
