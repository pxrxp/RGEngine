#ifndef SERVICE_LOCATOR_H
#define SERVICE_LOCATOR_H

#include <Graphics/Window/WindowManager.h>
#include <Graphics/Shaders/ShaderManager.h>
#include <Core/EntityManager.h>
#include <Core/ComponentManager.h>
#include <Core/SystemManager.h>
#include <Core/EventManager.h>

class ServiceLocator {
public:
	static EntityManager& get_entity_manager() {
		return EntityManager::get_instance();
	}

	static ComponentManager& get_component_manager() {
		return ComponentManager::get_instance();
	}

	static SystemManager& get_system_manager() {
		return SystemManager::get_instance();
	}

	static EventManager& get_event_manager() {
		return EventManager::get_instance();
	}

	static ShaderManager& get_shader_manager() {
		return ShaderManager::get_instance();
	}

	static WindowManager& get_window_manager() {
		return WindowManager::get_instance();
	}
};

#endif // SERVICE_LOCATOR_H
