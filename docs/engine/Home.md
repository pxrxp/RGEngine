# Engine

The engine lives in `engine/` and is built as the static library `engine`. It provides the window, input, camera, scene graph, renderer, resource repositories, collision queries, physics component, and asset utilities that the game is built on.

It contains no game content: no window title, no player, no post-process look, no HUD, and no asset names. The game supplies all of that through `AppConfig`, `Scene` subclasses, and its own shaders.

## Layout

```text
engine/
├── include/engine/     public headers (App, Window, Camera, Renderer, Scene,
│                       SceneObject, Mesh, Material, Texture, Shader,
│                       ShaderProgram, CollisionWorld, TriangleMeshCollider)
│   ├── components/     TransformComponent, PhysicsComponent, BoundingBox,
│   │                   vertex formats
│   ├── repos/          RepoBase, MeshRepo, ShaderRepo, ShaderProgramRepo,
│   │                   TextureRepo, MaterialRepo
│   └── util/           filesystem, PNG loading, OBJ loading, GL debug, editor
├── src/                implementation
└── shaders/            engine-owned shaders (AABB debug lines, skybox,
                        post-process vertex stage)
```

## Where to start

- [Architecture](Architecture.md) — how the pieces fit together
- [Application](Application.md) — `App`, `AppConfig`, frame flow, post-process hook
- [Window and input](Window-and-Input.md)
- [Scene system](Scene-System.md) and [scene objects](Scene-Objects.md)
- [Rendering](Rendering.md)
- [Shaders and shader programs](Shaders-and-Shader-Programs.md)
- [Textures and materials](Textures-and-Materials.md)
- [Meshes](Meshes.md)
- [Collision and physics](Collision-and-Physics.md)
- [Components](Components.md)
- [Resource repositories](Resource-Repositories.md)
- [Utilities](Utilities.md)
- [Extending the engine](Extending-the-Engine.md)
