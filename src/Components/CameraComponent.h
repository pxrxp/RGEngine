#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include <Core/ComponentManager.h>
#include <glm/glm.hpp>

struct CameraComponent : Component{
	void init_access() override {}

	glm::mat4 projection;
	glm::vec3 up;
};

#endif // CAMERA_COMPONENT_H
