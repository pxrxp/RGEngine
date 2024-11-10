#include <glad/gl.h>
#include <Graphics/Window/Window.h>

Window::Window(GLFWwindow* window_ptr, int width, int height, double last_time) :
	window{window_ptr},
	width{width},
	height{height},
	last_time{last_time},
	current_time{0.0},
	delta_time{0.0} {
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

int Window::get_width() const {
	return width;
}

int Window::get_height() const {
	return height;
}

float Window::get_aspect_ratio() const {
	return static_cast<float>(width) / static_cast<float>(height);
}

double Window::get_delta_time() const {
	return delta_time;
}

void Window::init_graphics() {
	current_time = glfwGetTime();
	delta_time = current_time - last_time;
	last_time = current_time;

	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool Window::is_open() const {
	return !glfwWindowShouldClose(window);
}

void Window::render() {
	glfwSwapBuffers(window);
	glfwPollEvents();
}
