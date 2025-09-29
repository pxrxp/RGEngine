#pragma once
#include "App.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneObject.h"

class TestScene : public Scene
{
public:
  TestScene(App& app)
    : Scene(app)
  {
  }

  TestScene(const TestScene&) = delete;
  TestScene& operator=(const TestScene&) = delete;

  TestScene(TestScene&&) = default;
  TestScene& operator=(TestScene&&) = delete;

  void init() override;

private:
  void setup_skybox(const SceneObjectStrongPtr& skybox);
  // void setup_cube(const SceneObjectStrongPtr& cube);
  // void setup_frustum(const SceneObjectStrongPtr& frustum);
  // void setup_objmodel(const SceneObjectStrongPtr& objmodel);
};
