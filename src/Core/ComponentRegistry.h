#ifndef COMPONENT_REGISTRY_H
#define COMPONENT_REGISTRY_H

#include <memory>
#include <unordered_map>
#include <type_traits>

using Entity = unsigned int;
class ComponentManager;

struct Component {
	virtual void init_access() = 0;
};

template <typename T>
concept DerivedFromComponent = std::is_base_of<Component, T>::value;

template <DerivedFromComponent ComponentType>
class ComponentRegistry {
private:
	std::unordered_map<Entity, std::unique_ptr<ComponentType>> _components;
	static ComponentRegistry& get_instance() {
		static ComponentRegistry instance;
		return instance;
	}

	void set_component(Entity id, std::unique_ptr<ComponentType> component) {
		_components[id] = std::move(component);
	}

	ComponentType* get_component(Entity id) {
		auto it = _components.find(id);
		if (it != _components.end()) {
			return it->second.get();
		}
		return nullptr;
	}

	ComponentRegistry() = default;
	ComponentRegistry(const ComponentRegistry&) = delete;
	ComponentRegistry& operator=(const ComponentRegistry&) = delete;

public:
	friend class ComponentManager;
};


#endif  // COMPONENT_REGISTRY_H
