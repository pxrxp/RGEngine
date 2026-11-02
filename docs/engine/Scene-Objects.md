# Scene objects

`SceneObject` (`engine/include/engine/SceneObject.h`) is the node type of the scene tree. It is not a general-purpose entity: it holds a transform, a physics component, an optional render packet, and children.

```cpp
auto object = std::make_shared<SceneObject>();
object->get_local_transform().set_position(pos).set_scale(scale);
object->get_physics_component().disable();
object->set_collidable(false);
object->set_render_packet(
  app.mesh_repo.load_mesh(desc),
  app.material_repo.load_material(material_desc),
  capture_weak(object, [&app](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
    app.get_scene_ptr(); // ...
    packet->material->get_shader_program()->set_uniform("u_model", self->get_world_transformation_mat());
    packet->mesh->draw();
  }));
parent->add_child(object);
```

## Members

| Member | Notes |
| --- | --- |
| `get_local_transform()` | `TransformComponent`; setters mark the node dirty. |
| `get_physics_component()` | `PhysicsComponent`; enabled by default, with gravity on. |
| `set_render_packet(args...)` | Builds a `RenderPacket{mesh, material, render, priority}`. All arguments are forwarded, so the overload is selected by the packet constructor. |
| `get_render_packet()` | `std::optional<RenderPacketStrongPtr>`; empty when the object is not drawable. |
| `visible` | Public flag; `capture_weak` respects it. |
| `set_collidable()` / `is_collidable()` | Controls whether `Scene::collect_colliders_recursive` includes the object. |
| `add_child()` / `remove_child()` / `get_parent()` | Parent is a weak pointer; `remove_child` detaches a node without destroying it; iteration over children is available through `begin()`/`end()`. |
| `get_world_AABB()` | Recomputes from the mesh's local AABB when the transform is dirty. |

## Render packets and priority

`RenderPacket` (`engine/include/engine/Renderer.h`) ties a mesh to a material and stores a `std::function<void()> render`. `Priority` is `HIGH_PRIORITY`, `NORMAL_PRIORITY` or `LOW_PRIORITY`; `LOW_PRIORITY` is drawn last, which is how a first-person viewmodel can be drawn over the world.

The mesh and material must be non-null (`assert`s in debug builds).

## World transform

`get_world_transformation_mat()` composes parent transforms recursively and normalises the parent's basis vectors so scaling does not accumulate down the tree. `get_local_transformation_mat()` returns the local model matrix.
