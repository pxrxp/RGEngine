#include <gtest/gtest.h>
#include <Core/EventManager.h>
#include <Core/ServiceLocator.h>
#include <Events/InputEvent.h>

class TestEventListener : public EventListener {
public:
    void listen(const InputEvent& event) override {
        _listened = true;
    }

    bool _listened = false;
};

class EventManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        EventManager& manager = ServiceLocator::get_event_manager();
        manager.reset();
    }
};

TEST_F(EventManagerTest, SingletonTest) {
    EventManager& instance1 = ServiceLocator::get_event_manager();
    EventManager& instance2 = ServiceLocator::get_event_manager();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(EventManagerTest, TriggerEventTest) {
    EventManager& manager = ServiceLocator::get_event_manager();
    InputEvent& event = InputEvent::get_instance();
    TestEventListener listener;

    manager.subscribe(event, listener);
    manager.trigger_event(event);

    EXPECT_TRUE(listener._listened);
}

TEST_F(EventManagerTest, SubscribeMultipleListenersTest) {
    EventManager& manager = ServiceLocator::get_event_manager();
    InputEvent& event = InputEvent::get_instance();
    TestEventListener listener1;
    TestEventListener listener2;

    manager.subscribe(event, listener1);
    manager.subscribe(event, listener2);
    manager.trigger_event(event);

    EXPECT_TRUE(listener1._listened);
    EXPECT_TRUE(listener2._listened);
}

TEST_F(EventManagerTest, SubscribeMultipleEventsTest) {
    EventManager& manager = ServiceLocator::get_event_manager();
    InputEvent& event1 = InputEvent::get_instance();
    InputEvent& event2 = InputEvent::get_instance();
    TestEventListener listener;

    manager.subscribe(event1, listener);
    manager.subscribe(event2, listener);
    manager.trigger_event(event1);
    manager.trigger_event(event2);

    EXPECT_TRUE(listener._listened);
}

TEST_F(EventManagerTest, TriggerEventMultipleTimesTest) {
    EventManager& manager = ServiceLocator::get_event_manager();
    InputEvent& event = InputEvent::get_instance();
    TestEventListener listener;

    manager.subscribe(event, listener);
    manager.trigger_event(event);
    manager.trigger_event(event);

    EXPECT_TRUE(listener._listened);
}
