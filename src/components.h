#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct TransformComponent {
  glm::vec3 position = glm::vec3(0.0f);
  glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale_factors = glm::vec3(1.0f);
};

struct PhysicsComponent {
  bool obey_gravity;
  glm::vec3 speed;
  float mass = 1.0f;
};
