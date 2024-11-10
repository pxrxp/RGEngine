#ifndef KINEMATICS_COMPONENT_H
#define KINEMATICS_COMPONENT_H

#include <Core/ComponentManager.h>
#include <glm/glm.hpp>

struct KinematicsComponent : Component {
	void init_access() override {}

	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 damping;
};

#endif // KINEMATICS_COMPONENT_H
