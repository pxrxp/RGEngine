# Coding conventions

The project uses `.clang-format` with `BasedOnStyle: Mozilla` and `AlignConsecutiveMacros: true`. Format with clang-format before committing.

## Names

- Types: `PascalCase` (`SceneObject`, `MaterialRepo`, `Vertex_PosTexNorm`).
- Functions and variables: `snake_case` (`load_material`, `m_delta_time`, `update_physics`).
- Private members: `m_` prefix; static members: `s_` prefix (`s_next_id`).
- Constants: `SCREAMING_SNAKE_CASE` (`FIXED_STEP`, `CITY_MODEL_SCALE`, `PLAYER_RADIUS`).
- Macros: `SCREAMING_SNAKE_CASE` (`SETUP_ATTRIB`, `VERTEX_PTN_FIELDS`).

## Formatting

- Two-space indentation, 80-column target.
- Function return type on its own line followed by `Class::function(...)`; opening brace on the next line.
- Pointer/reference asterisk binds to the type (`SceneObject&`, `int*`).
- Preprocessor guards use `#ifndef NDEBUG` for debug-only code.

## Structure

- Headers in `include/`, implementations in `src/`, mirrored paths.
- One class per header, header-named `.cpp` for its implementation. The game's `RivalGrounds` is the deliberate exception: one large header plus a separate debug-menu source.
- Public engine headers should include only what they need; prefer forward declarations in headers and full includes in `.cpp` files.

## Design habits used in this codebase

- `std::shared_ptr` for scene objects and repositories; `std::weak_ptr` for parents and render callbacks (`capture_weak`); no raw owning pointers.
- Descriptor structs with designated initialisers for loaders (`MaterialDescriptor{ .material_name = ... }`).
- Loaders return `{}`/`nullptr` and log instead of throwing, except `Util::read_file*` which throws on I/O failure and is caught by callers that can recover.
- Engine code must not include `rivalgrounds/` headers or contain game strings or values.
- UI code lives next to the state it displays (`render_game_hud`, `render_debug_menu`).
