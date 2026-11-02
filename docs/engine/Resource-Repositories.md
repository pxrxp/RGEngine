# Resource repositories

All GPU resources are cached in repositories keyed by name.

## RepoBase

`engine/include/engine/repos/RepoBase.h` is a small template:

```cpp
template<typename Key, typename Resource>
class RepoBase
{
protected:
  std::unordered_map<Key, std::shared_ptr<Resource>> m_resources;
  template<typename... Args>
  std::shared_ptr<Resource> create(Key key, Args&&... args);
public:
  std::optional<std::shared_ptr<Resource>> get(const Key& id);
  void remove(const Key& id);
  void clear();
};
```

`create()` stores and returns a `shared_ptr`. Subclass loaders check `get()` first, so a name is loaded once per session.

## Repository tree

```text
App
├── mesh_repo : MeshRepo
└── material_repo : MaterialRepo
    ├── tex_repo : TextureRepo
    └── shader_program_repo : ShaderProgramRepo
        └── shader_repo : ShaderRepo
```

Use `app.material_repo.tex_repo` for textures that materials will reference. A material looks its textures up in that same repository, so using one texture cache keeps a single GL object per file.

## Implementations

| Repository | Key | Loader |
| --- | --- | --- |
| `MeshRepo` | mesh name | `load_mesh(desc, reload = false)` |
| `ShaderRepo` | shader name (`cube.vert`) | `load_shader(name)` |
| `ShaderProgramRepo` | program name | `load_program(desc)` |
| `TextureRepo` | texture name | `load_texture(desc)` |
| `MaterialRepo` | material name | `load_material(desc)` |

## Cleanup

`App::cleanup()` clears, in order: the renderer queue, the shader repository, the shader program repository, the texture repository, and the material repository; then the mesh repository. Clear materials and meshes before destroying the GL context — `App`'s destructor destroys the scene framebuffer and ShaderPrograms, while `Window`'s destructor terminates GLFW.
