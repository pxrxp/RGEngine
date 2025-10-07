#pragma once
#include <string>

class TransformComponent;
class PhysicsComponent;

#ifndef NDEBUG
namespace Util {

void
draw_transform_component_editor(TransformComponent& component, const std::string& header);

void
draw_physics_component_editor(PhysicsComponent& component, const std::string& header);

}

#endif
