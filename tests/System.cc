#include <gtest/gtest.h>
#include <Core/SystemManager.h>
#include <Core/ServiceLocator.h>

class TestSystem : public System {
public:
    void update() override {
        _updated = true;
    }

    void register_events() override {
        _registered = true;
    }

    bool _updated = false;
    bool _registered = false;
};

class SystemManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        SystemManager& manager = ServiceLocator::get_system_manager();
        manager.reset();
    }
};

TEST_F(SystemManagerTest, SingletonTest) {
    SystemManager& instance1 = ServiceLocator::get_system_manager();
    SystemManager& instance2 = ServiceLocator::get_system_manager();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(SystemManagerTest, AddSystemTest) {
    SystemManager& manager = ServiceLocator::get_system_manager();
    TestSystem system;

    manager.add(system);

    manager.update();
    EXPECT_TRUE(system._updated);
}

TEST_F(SystemManagerTest, UpdateSystemsTest) {
    SystemManager& manager = ServiceLocator::get_system_manager();
    TestSystem system1;
    TestSystem system2;

    manager.add(system1);
    manager.add(system2);

    manager.update();

    EXPECT_TRUE(system1._updated);
    EXPECT_TRUE(system2._updated);
}

TEST_F(SystemManagerTest, RegisterEventsTest) {
    SystemManager& manager = ServiceLocator::get_system_manager();
    TestSystem system1;
    TestSystem system2;

    manager.add(system1);
    manager.add(system2);

    manager.register_events();

    EXPECT_TRUE(system1._registered);
    EXPECT_TRUE(system2._registered);
}

TEST_F(SystemManagerTest, UpdateMultipleTimesTest) {
    SystemManager& manager = ServiceLocator::get_system_manager();
    TestSystem system;

    manager.add(system);

    manager.update();
    manager.update();

    EXPECT_TRUE(system._updated);
}

TEST_F(SystemManagerTest, RegisterEventsMultipleTimesTest) {
    SystemManager& manager = ServiceLocator::get_system_manager();
    TestSystem system;

    manager.add(system);

    manager.register_events();
    manager.register_events();

    EXPECT_TRUE(system._registered);
}

