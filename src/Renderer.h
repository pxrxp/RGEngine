#pragma once
#include <vector>
#include <functional>

#include "Mesh.h"
#include "Material.h"
#include "Shader.h"

class App;

struct RenderPacket {
  GLuint vao;
  GLuint shader;
  unsigned int material;
  std::function<void()> render;

  RenderPacket(Mesh& mesh, ShaderProgram& shader, Material& material)
      : vao(mesh.get_id()), shader(shader.get_id()), material(material.get_id()) {}
};

class Renderer {
  std::vector<RenderPacket> render_queue;

public:
  void add(RenderPacket render_packet);
  void render();
};
