#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H

#include <Core/ComponentRegistry.h>

class ComponentManager {
private:
	ComponentManager() = default;
public:
	template <DerivedFromComponent ComponentType>
	ComponentType* get_component(Entity id) {
		auto& component_registry = ComponentRegistry<ComponentType>::get_instance();
		return component_registry.get_component(id);
	}

	template <DerivedFromComponent ComponentType>
	ComponentManager& set_component(Entity id, std::unique_ptr<ComponentType> component) {
		auto& component_registry = ComponentRegistry<ComponentType>::get_instance();
		component_registry.set_component(id, std::move(component));
		return *this;
	}

	static ComponentManager& get_instance() {
		static ComponentManager instance;
		return instance;
	}

	ComponentManager(const ComponentManager&) = delete;
	ComponentManager& operator=(const ComponentManager&) = delete;
};

#endif // COMPONENT_MANAGER_H
