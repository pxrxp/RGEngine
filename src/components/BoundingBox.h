#pragma once
#include "vertex_formats.h"
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

enum Where { Outside, Inside, AtBoundary };

struct BoundingBox {
  glm::vec3 min;
  glm::vec3 max;

  BoundingBox();
  explicit BoundingBox(const std::vector<SimpleVertex> &points);

  Where where(const glm::vec3 &point) const;
  bool collides(const BoundingBox &other) const;
  BoundingBox &add_point(const glm::vec3 &point);
  std::vector<SimpleVertex> corners() const;
  std::vector<LineIndices> indices() const;
};

#ifndef NDEBUG
std::ostream &operator<<(std::ostream &os, const BoundingBox &bb);
#endif
