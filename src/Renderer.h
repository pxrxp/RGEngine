#pragma once
#include <functional>
#include <memory>
#include <vector>

class App;
class Mesh;
class ShaderProgram;
class Material;

enum Priority {
  HIGH_PRIORITY = 0,
  NORMAL_PRIORITY = 1,
  LOW_PRIORITY = 2,
};

struct RenderPacket {
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<ShaderProgram> shader_program;
  std::shared_ptr<Material> material;
  Priority priority;

  std::function<void()> render;

  RenderPacket(std::shared_ptr<Mesh> mesh,
               std::shared_ptr<ShaderProgram> shader,
               std::shared_ptr<Material> material, Priority priority = NORMAL_PRIORITY)
      : mesh(mesh), shader_program(shader), material(material) {}
};

class Renderer {
  std::vector<RenderPacket> render_queue;

public:
  void add(RenderPacket render_packet);
  void render();
};
