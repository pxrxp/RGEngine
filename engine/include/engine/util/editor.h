#pragma once
#include <string>

class TransformComponent;

#ifndef NDEBUG
namespace Util {

void
draw_transform_component_editor(TransformComponent& component,
                                const std::string& header);

}
#endif
