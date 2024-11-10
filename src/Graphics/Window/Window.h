#ifndef WINDOW_H
#define WINDOW_H
#include <GLFW/glfw3.h>

class WindowManager;

class Window {
private:
	GLFWwindow* window;
	int height;
	int width;

	double last_time;
	double current_time;
	double delta_time;

	Window(GLFWwindow* window_ptr, int width, int height, double last_time);

public:
	friend class WindowManager;

	int get_width() const;
	int get_height() const;
	float get_aspect_ratio() const;
	double get_delta_time() const;

	void init_graphics();
	bool is_open() const;
	void render();

	Window(const Window&) = default;
	~Window();
};

#endif // WINDOW_H
