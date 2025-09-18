#include "App.h"
#include "Colors.h"
#include "Mesh.h"
#include "Shader.h"
#include <glm/trigonometric.hpp>
#include <vector>

int main() {
  App app;
  app.init_window();

  ShaderProgram program;
  program.load_shaders({"triangle.vert", "triangle.frag"});

  Mesh triangle;
  triangle.setup(
      {
          -0.5f, -0.5f, 0.0f, // left
          0.5f, -0.5f, 0.0f,  // right
          0.0f, 0.5f, 0.0f    // top
      },
      {}, {3});

  float angle = 0;

  while (app.is_running()) {
    glfwPollEvents();
    app.update();

    app.get_window().clear(COLOR_MAROON, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program.bind();
    triangle.draw();
    program.unbind();

    app.get_window().swap_buffers();
  }

  app.terminate_window();
}
