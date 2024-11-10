#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include <Core/EventManager.h>
#include <Core/ServiceLocator.h>

#include <Events/InputEvent.h>

class CameraSystem : public System, public EventListener {
public:
	void listen(const InputEvent& event) override;
	void update() override;
	void register_events() override;
};

#endif // CAMERA_SYSTEM_H
