#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Core/EventManager.h>
#include <functional>
#include <vector>

struct System {
	virtual void update() = 0;
	virtual void register_events() = 0;
};

class ServiceLocator;

class SystemManager {
private:
	std::vector<std::reference_wrapper<System>> _systems;
	static SystemManager& get_instance();

	SystemManager() = default;
public:
	friend class ServiceLocator;

	SystemManager& add(System& system);
	void register_events() const;
	void update() const;

	SystemManager(const SystemManager&) = delete;
	SystemManager& operator=(const SystemManager&) = delete;

	#ifndef NDEBUG
	void reset();
	#endif
};

#endif // SYSTEM_MANAGER_H
