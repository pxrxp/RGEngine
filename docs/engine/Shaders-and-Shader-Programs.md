# Shaders and shader programs

## Files

Shaders are loaded from `shaders/<name>.glsl` relative to the executable. The stage is inferred from the last character of the descriptor entry:

| Suffix | Stage |
| --- | --- |
| `.vert` | `GL_VERTEX_SHADER` |
| `.frag` | `GL_FRAGMENT_SHADER` |
| `.geo` | `GL_GEOMETRY_SHADER` |
| `.comp` | `GL_COMPUTE_SHADER` |
| `.ctrl` | `GL_TESS_CONTROL_SHADER` |
| `.eva` | `GL_TESS_EVALUATION_SHADER` |

`ShaderRepo::load_shader(name)` reads `shaders/<name>.glsl` through `Util::read_file`, compiles it, prints the info log on failure, and caches the result by name.

## Programs

```cpp
ShaderProgramDescriptor desc{
  .program_name = "cube_program",
  .shaders = { "cube.vert", "cube.frag" }
};
auto program = app.material_repo.shader_program_repo.load_program(desc);
```

`ShaderProgramRepo::load_program()` creates and links a program, attaches the requested shaders, prints the link log on failure, and caches by `program_name`. Loading the same name again returns the cached program.

## Uniforms

`ShaderProgram` exposes typed setters:

```cpp
program->set_uniform("u_color", glm::vec3(1.0f));
program->set_uniform("u_use_texture", 1);
program->set_uniform("u_model", matrix);
```

Supported types: `float`, `int`, `glm::vec2`, `glm::vec3`, `glm::vec4`, `glm::mat3`, `glm::mat4`. Setters use `glGetUniformLocation` plus `glProgramUniform*`, so they work without binding the program. A missing uniform logs a warning and is ignored.

`program->bind()` calls `glUseProgram(program->get_id())`.

## Lifecycle

`Shader` and `ShaderProgram` delete their GL objects in their destructors; shader objects can therefore be dropped after linking (the repositories keep them cached for the session).

## Engine and game shaders

- `engine/shaders`: `AABB.*` (debug lines), `skybox.*`, `postprocess.vert`.
- `game/shaders`: `cube.*` (the game's material shader) and `grayscale.frag` (the game's post-process).

Both directories are copied next to the executable by a CMake post-build step.
