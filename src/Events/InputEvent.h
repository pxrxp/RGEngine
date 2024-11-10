#ifndef INPUT_EVENT_H
#define INPUT_EVENT_H

#include <Core/EventManager.h>

struct InputEvent : Event {
	void init_access() override {}

	static InputEvent& get_instance() {
		static InputEvent instance;
		return instance;
	}

	bool forward: 1;
	bool left: 1;
	bool backward: 1;
	bool right: 1;

	bool sneak: 1;
	bool sprint: 1;

	bool attack: 1;
	bool open: 1;
	bool jump: 1;

	double mouse_x;
	double mouse_y;
};

#endif // INPUT_EVENT_H
