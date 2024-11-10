#include <Core/ServiceLocator.h>
#include <Systems/CameraSystem.h>

void CameraSystem::listen(const InputEvent& event) {
}

void CameraSystem::update(){

}

void CameraSystem::register_events() {
	ServiceLocator::get_event_manager()
		.subscribe(InputEvent::get_instance(), *this);
}
