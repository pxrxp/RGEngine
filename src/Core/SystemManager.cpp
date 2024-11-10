#include <Core/SystemManager.h>

SystemManager& SystemManager::get_instance() {
	static SystemManager instance;
	return instance;
}

SystemManager& SystemManager::add(System& system) {
	_systems.push_back(system);
	return *this;
}

void SystemManager::update() const {
	for (auto& system : _systems) {
		system.get().update();
	}
}

void SystemManager::register_events() const {
	for (auto& system : _systems) {
		system.get().register_events();
	}
}

#ifndef NDEBUG
void SystemManager::reset() {
	_systems.clear();
}
#endif
