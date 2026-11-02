# Components

The engine has three component types. There is no entity/component registry: components are members of `SceneObject` or plain values.

## TransformComponent

`engine/include/engine/components/TransformComponent.h`

- state: position (`vec3`), rotation (`quat`), scale (`vec3`), dirty flag and cached model matrix;
- setters: `set_position`, `set_rotation` (normalises), `set_scale`, `translate`, `rotate(axis, radians)`, `rotate_deg(axis, degrees)`, `scale(factors)`;
- getters: `get_position`, `get_rotation`, `get_scale`, `get_model_matrix()` which rebuilds `T * R * S` on demand.

Every mutating call marks the transform dirty, which invalidates cached world matrices and AABBs.

## PhysicsComponent

`engine/include/engine/components/PhysicsComponent.h` — see [Collision and physics](Collision-and-Physics.md). Constructed with the transform it owns and the fixed step; enabled by default.

## BoundingBox

`engine/include/engine/components/BoundingBox.h` — an axis-aligned box with ray intersection and dirty tracking. Used for world AABBs, colliders, enemy hitboxes, and debug visualisation.

## Vertex formats

`engine/include/engine/components/vertex_formats.h` defines the vertex structs and index types used by meshes, plus debug `operator<<` helpers. The structs are generated from field macros so `MeshRepo` can configure attributes in one place.

## Editor helpers (debug only)

`engine/include/engine/util/editor.h` exposes `Util::draw_transform_component_editor(component, header)` in debug builds: drag-float fields for position, rotation in degrees, and scale. It is used by `Scene::debug_camera()`.
