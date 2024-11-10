#define GLAD_GL_IMPLEMENTATION
#define GLFW_INCLUDE_NONE
#include <glad/gl.h>
#include <iostream>

#include <Graphics/Window/WindowManager.h>
#include <Graphics/Window/Callbacks.h>
#include <Graphics/ErrorHandling.h>

constexpr int MIN_OPENGL_VERSION_MAJOR = 3;
constexpr int MIN_OPENGL_VERSION_MINOR = 3;

WindowManager& WindowManager::get_instance() {
	static WindowManager instance;
	return instance;
}

WindowManager::WindowManager() {
	glfwSetErrorCallback(windowing_error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MIN_OPENGL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MIN_OPENGL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	#ifndef NDEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	#endif
}

Window WindowManager::create_window(
	const char* title,
	GLFWmonitor* monitor,
	const WindowSettings dimensions,
	int cursor_mode,
	int swap_interval,
	GLFWwindow* share_resources
) {
	GLFWwindow* window;
	GLFWmonitor* monitor_for_full_screen;
	int width, height;
	double last_time;


	if (dimensions == FULL_SCREEN) {
		auto video_mode = glfwGetVideoMode(monitor);
		width = video_mode->width;
		height = video_mode->height;
		monitor_for_full_screen = monitor;

	} else if (dimensions == DEFAULT_DIMENSIONS) {
		auto video_mode = glfwGetVideoMode(monitor);
		width = video_mode->width/2;
		height = video_mode->height/2;
		monitor = nullptr;

	} else {
		width = dimensions.width;
		height = dimensions.height;
		monitor = nullptr;
	}

	window = glfwCreateWindow(width, height, title, monitor, share_resources);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);

	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		std::cerr << "Error: Failed to initialize GLAD" << std::endl;
		exit(EXIT_FAILURE);
	}

	glfwSwapInterval(swap_interval);

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(gl_debug_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Initialized debug context");
	}

	last_time = glfwGetTime();

	return Window(window, width, height, last_time);
}
