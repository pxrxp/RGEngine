#pragma once
#include <cassert>
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

class App;
class Mesh;
class ShaderProgram;
class Material;

enum Priority
{
  HIGH_PRIORITY = 0,
  NORMAL_PRIORITY = 1,
  LOW_PRIORITY = 2,
};

struct RenderPacket
{
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
  Priority priority;

  std::function<void()> render;

  RenderPacket(std::shared_ptr<Mesh> mesh,
               std::shared_ptr<Material> material,
               std::function<void()> render_function,
               Priority priority = NORMAL_PRIORITY)
    : mesh(std::move(mesh))
    , material(std::move(material))
    , priority(priority)
    , render(std::move(render_function))
  {
    assert(this->mesh != nullptr && "A RenderPacket must have a valid Mesh");
    assert(this->material != nullptr && "A RenderPacket must have a valid Material");
  }

  ~RenderPacket() = default;
};

using RenderPacketStrongPtr = std::shared_ptr<RenderPacket>;
using RenderPacketWeakPtr = std::weak_ptr<RenderPacket>;

class SceneObject;
using SceneObjectStrongPtr = std::shared_ptr<SceneObject>;

class Renderer
{
  std::vector<RenderPacketStrongPtr> m_render_queue;

public:
  void add(RenderPacketStrongPtr render_packet);
  void add_children(SceneObjectStrongPtr root,
                    std::unordered_set<SceneObjectStrongPtr>& set);
  void submit(SceneObjectStrongPtr scene);
  void render();
  void clear();
  ~Renderer() = default;
};
