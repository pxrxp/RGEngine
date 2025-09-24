#pragma once
#ifndef NDEBUG
#define GLM_ENABLE_EXPERIMENTAL
#include <chrono>
#include <glm/gtx/io.hpp>
#include <imgui.h>
#include <iostream>
#include <thread>
#include <vector>

#define SLEEP(X) std::this_thread::sleep_for(std::chrono::seconds(X))

template<typename T>
inline std::ostream&
operator<<(std::ostream& os, const std::vector<T>& vec)
{
  os << "[";
  for (size_t i = 0; i < vec.size(); ++i) {
    os << vec[i];
    if (i + 1 < vec.size())
      os << ", ";
  }
  os << "]";
  return os;
}
#endif
