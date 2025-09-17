#include "App.h"
#include "Colors.h"
#include "Shader.h"
#include <glm/trigonometric.hpp>

int main() {
  App app;
  app.init_window();


  float vertices[] = {
      -0.5f, -0.5f, 0.0f,  // left
       0.5f, -0.5f, 0.0f,  // right
       0.0f,  0.5f, 0.0f   // top
  };

  ShaderProgram program;
  program.load_shaders({"triangle.vert", "triangle.frag"});

  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  float angle = 0;

  while (app.is_running()) {
    glfwPollEvents();
    app.update();

    app.get_window().clear(PINK, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program.bind();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    program.unbind();

    app.get_window().swap_buffers();
  }

  app.terminate_window();
}
