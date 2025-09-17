#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto primary_monitor = glfwGetPrimaryMonitor();
  auto video_mode = glfwGetVideoMode(primary_monitor);

  const int DEFAULT_WIDTH = video_mode->width / 2;
  const int DEFAULT_HEIGHT = video_mode->height / 2;
  const auto WINDOW_TITLE = "RivalGrounds";

  GLFWwindow *window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT,
                                        WINDOW_TITLE, nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    glViewport(0, 0, 800, 600);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
