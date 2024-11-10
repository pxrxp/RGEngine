#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <utility>

using Entity = unsigned int;
constexpr unsigned int MAX_ENTITIES = 10000;

class EntityManager {
private:
	unsigned int _next_id;
	static EntityManager& get_instance();

	EntityManager() : _next_id{1} {}
public:
	friend class ServiceLocator;

	Entity create_entity();
	std::pair<Entity, Entity> create_entities(unsigned int count);

	EntityManager(const EntityManager&) = delete;
	EntityManager& operator=(const EntityManager&) = delete;

	#ifndef NDEBUG
	void reset();
	#endif
};

#endif // ENTITY_MANAGER_H
