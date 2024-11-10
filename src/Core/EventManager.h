#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <typeindex>
#include <unordered_map>
#include <functional>
#include <set>

#include <Core/EventListener.h>

class ServiceLocator;

struct Event {
	virtual void init_access() = 0;
};

template <typename T>
concept DerivedFromEvent = std::is_base_of<Event, T>::value;

class EventManager {
private:
	struct EventListenerOverloads {
		bool operator()(
			const std::reference_wrapper<EventListener>& lhs,
			const std::reference_wrapper<EventListener>& rhs
		) const {
			return &lhs.get() < &rhs.get();
		}
	};

	std::unordered_map<
		std::type_index,
		std::set<std::reference_wrapper<EventListener>, EventListenerOverloads>
	> _subscribers;

	EventManager() = default;

	static EventManager& get_instance() {
		static EventManager instance;
		return instance;
	}

public:
	friend class ServiceLocator;

	template <typename DerivedFromEvent>
	EventManager& trigger_event(const DerivedFromEvent& event) {
		auto it = _subscribers.find(typeid(event));
		if (it == _subscribers.end()) return *this;

		auto& listeners = it->second;

		for (auto& listener : listeners)
			listener.get().listen(event);
		return *this;
	}

	template <typename DerivedFromEvent>
	EventManager& subscribe(const DerivedFromEvent& event, EventListener& listener) {
		std::type_index event_type = typeid(event);

		auto it = _subscribers.find(event_type);
		if (it == _subscribers.end())
			_subscribers[event_type] = std::set<std::reference_wrapper<EventListener>, EventListenerOverloads>();

		_subscribers[event_type].insert(listener);

		return *this;
	}

	EventManager(const EventManager&) = delete;
	EventManager& operator=(const EventManager&) = delete;

	#ifndef NDEBUG
	void reset() {
		_subscribers.clear();
	}
	#endif
};

#endif // EVENT_MANAGER_H
