#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <Core/ComponentManager.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct TransformComponent : Component {
	void init_access() override {}

	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
};

#endif // TRANSFORM_COMPONENT_H
