#include <Graphics/Window/Callbacks.h>
#include <Events/InputEvent.h>
#include <Core/ServiceLocator.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto input_event = InputEvent::get_instance();
	bool pressed = action != GLFW_RELEASE;

	switch (key) {
	case GLFW_KEY_W:
	case GLFW_KEY_UP:
		input_event.forward = pressed;
		break;
	
	case GLFW_KEY_A:
	case GLFW_KEY_LEFT:
		input_event.left = pressed;
		break;

	case GLFW_KEY_S:
	case GLFW_KEY_DOWN:
		input_event.backward = pressed;
		break;

	case GLFW_KEY_D:
	case GLFW_KEY_RIGHT:
		input_event.right = pressed;
	
	case GLFW_KEY_SPACE:
		input_event.jump = pressed;
		break;

	case GLFW_KEY_F:
		input_event.open = pressed;
		break;
	
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	}

	switch(mods) {
	case GLFW_MOD_SHIFT:
		input_event.sneak = pressed;
		break;
	
	case GLFW_MOD_CONTROL:
		input_event.sprint = pressed;
		break;
	}

	ServiceLocator::get_event_manager()
		.trigger_event<InputEvent>(input_event);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	auto input_event = InputEvent::get_instance();

	input_event.mouse_x = xpos;
	input_event.mouse_y = ypos;

	ServiceLocator::get_event_manager()
		.trigger_event<InputEvent>(input_event);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	auto input_event = InputEvent::get_instance();
	bool pressed = action != GLFW_RELEASE;

	input_event.attack = pressed;

	ServiceLocator::get_event_manager()
		.trigger_event<InputEvent>(input_event);
}
