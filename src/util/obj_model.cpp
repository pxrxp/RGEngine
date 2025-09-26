#include "filesystem.h"
#include <sstream>
#include <iostream>

int main() {
  std::string path = "";
  std::string obj = Util::read_file("resources/" + path);

  std::stringstream ss(obj);
  std::cout << ss.str();
}

  // template<typename Vertex, typename Indices>
  // void load(const std::vector<Vertex>& vertices,
  //           const std::vector<Indices>& indices = {},
  //           GLenum draw_primitive = GL_TRIANGLES,
  //           GLenum usage = GL_STATIC_DRAW)
