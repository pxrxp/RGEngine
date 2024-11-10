#include <Core/EntityManager.h>

EntityManager& EntityManager::get_instance() {
	static EntityManager instance;
	return instance;
}

Entity EntityManager::create_entity() {
	if (_next_id > MAX_ENTITIES) return 0;
	return _next_id++;
}

std::pair<Entity, Entity> EntityManager::create_entities(unsigned int count) {
	if(count == 0) return std::make_pair(0,0);

	Entity first = _next_id;
	_next_id = first + count;

	if (_next_id > MAX_ENTITIES) _next_id = MAX_ENTITIES+1;
	return std::make_pair(first, _next_id-1);
}

#ifndef NDEBUG
void EntityManager::reset() {
	_next_id = 1;
}
#endif
