#include "App.h"
#include "Colors.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

std::string read_file(const char *path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Failed to open " << path << "\n";
    exit(EXIT_FAILURE);
  }

  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

int main() {
  App app;
  app.init_window();

  // Vertex shader
  std::string vertSrc = read_file("shader.vert");
  const char *vertCStr = vertSrc.c_str();
  GLuint vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vertCStr, nullptr);
  glCompileShader(vert);

  GLint success;
  glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info[512];
    glGetShaderInfoLog(vert, 512, nullptr, info);
    std::cerr << "Vertex shader error:\n" << info << "\n";
  }

  // Fragment shader
  std::string fragSrc = read_file("shader.frag");
  const char *fragCStr = fragSrc.c_str();
  GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &fragCStr, nullptr);
  glCompileShader(frag);

  glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info[512];
    glGetShaderInfoLog(frag, 512, nullptr, info);
    std::cerr << "Fragment shader error:\n" << info << "\n";
  }

  // Link program
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vert);
  glAttachShader(shaderProgram, frag);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char info[512];
    glGetProgramInfoLog(shaderProgram, 512, nullptr, info);
    std::cerr << "Program link error:\n" << info << "\n";
  }

  glDeleteShader(vert);
  glDeleteShader(frag);

  float vertices[] = {
      -0.5f, -0.5f, 0.0f,  // left
       0.5f, -0.5f, 0.0f,  // right
       0.0f,  0.5f, 0.0f   // top
  };

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

  while (app.is_running()) {
    glfwPollEvents();

    app.update();


    app.get_window().clear(PINK, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // RENDER

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    app.get_window().swap_buffers();
  }

  app.terminate_window();
}
