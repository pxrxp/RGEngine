#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <Graphics/Window/Window.h>

class ServiceLocator;

struct WindowSettings {
	int width;
	int height;

	bool operator==(const WindowSettings& other) const {
		return (width == other.width) && (height == other.height);
	}
};

const WindowSettings FULL_SCREEN = {0, 0};
const WindowSettings DEFAULT_DIMENSIONS = {-1, -1};
const int VSYNC = 1;

class WindowManager {
private:
	WindowManager();
	static WindowManager& get_instance();
public:
	friend class ServiceLocator;

	Window create_window(
		const char* title,
		GLFWmonitor* monitor,
		const WindowSettings width_height,
		int cursor_mode = GLFW_CURSOR_DISABLED,
		int swap_interval = VSYNC,
		GLFWwindow* share_resources = nullptr
	);
};

#endif // WINDOW_MANAGER_H
