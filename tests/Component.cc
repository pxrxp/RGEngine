#include <gtest/gtest.h>
#include <Core/ComponentManager.h>

TEST(ComponentManagerTest, SetComponentWithSameIdAndTypeMultipleTimes) {
    struct TestComponent : public Component {
        void init_access() override {}
    };

    ComponentManager& manager = ComponentManager::get_instance();
    Entity id = 1;
    std::unique_ptr<TestComponent> component1(new TestComponent());
    std::unique_ptr<TestComponent> component2(new TestComponent());
    manager.set_component(id, std::move(component1));
    manager.set_component(id, std::move(component2));

    TestComponent* retrievedComponent = manager.get_component<TestComponent>(id);
    EXPECT_NE(nullptr, retrievedComponent);
    EXPECT_EQ(retrievedComponent, manager.get_component<TestComponent>(id));
}

// Should cause compile error
/*TEST(ComponentManagerTest, SetComponentWithDifferentIdsAndTypes) {*/
/*    struct TestComponent1 : public Component {*/
/*        void init_access() override {}*/
/*    };*/
/**/
/*    struct TestComponent2 : public Component {*/
/*        void init_access() override {}*/
/*    };*/
/**/
/*    ComponentManager& manager = ComponentManager::get_instance();*/
/*    Entity id1 = 1;*/
/*    Entity id2 = 2;*/
/*    std::unique_ptr<TestComponent1> component1(new TestComponent1());*/
/*    std::unique_ptr<TestComponent2> component2(new TestComponent2());*/
/*    manager.set_component(id1, std::move(component1));*/
/*    manager.set_component(id2, std::move(component2));*/
/**/
/*    TestComponent1* retrievedComponent1 = manager.get_component<TestComponent1>(id1);*/
/*    TestComponent2* retrievedComponent2 = manager.get_component<TestComponent2>(id2);*/
/*    EXPECT_NE(nullptr, retrievedComponent1);*/
/*    EXPECT_NE(nullptr, retrievedComponent2);*/
/*    EXPECT_NE(retrievedComponent1, retrievedComponent2);*/
/*}*/

TEST(ComponentManagerTest, SetComponentWithSameIdAndTypeButDifferentInstancesMultipleTimes) {
    struct TestComponent : public Component {
        void init_access() override {}
    };

    ComponentManager& manager = ComponentManager::get_instance();
    Entity id = 1;
    std::unique_ptr<TestComponent> component1(new TestComponent());
    std::unique_ptr<TestComponent> component2(new TestComponent());
    std::unique_ptr<TestComponent> component3(new TestComponent());
    manager.set_component(id, std::move(component1));
    manager.set_component(id, std::move(component2));
    manager.set_component(id, std::move(component3));

    TestComponent* retrievedComponent = manager.get_component<TestComponent>(id);
    EXPECT_NE(nullptr, retrievedComponent);
    EXPECT_EQ(retrievedComponent, manager.get_component<TestComponent>(id));
}

// Should cause compile error
/*TEST(ComponentManagerTest, SetComponentWithNullptrMultipleTimes) {*/
/*    struct TestComponent : public Component {*/
/*        void init_access() override {}*/
/*    };*/
/**/
/*    ComponentManager& manager = ComponentManager::get_instance();*/
/*    Entity id = 1;*/
/*    manager.set_component(id, nullptr);*/
/*    manager.set_component(id, nullptr);*/
/**/
/*    TestComponent* retrievedComponent = manager.get_component<TestComponent>(id);*/
/*    EXPECT_EQ(nullptr, retrievedComponent);*/
/*}*/

// Should cause compile error
TEST(ComponentManagerTest, GetComponentWithInvalidIdMultipleTimes) {
    struct TestComponent : public Component {
        void init_access() override {}
    };

    ComponentManager& manager = ComponentManager::get_instance();
    Entity id = 0; // Invalid id
    TestComponent* retrievedComponent1 = manager.get_component<TestComponent>(id);
    TestComponent* retrievedComponent2 = manager.get_component<TestComponent>(id);

    EXPECT_EQ(nullptr, retrievedComponent1);
    EXPECT_EQ(nullptr, retrievedComponent2);
}

TEST(ComponentManagerTest, SetAndGetComponentWithMaxIdMultipleTimes) {
    struct TestComponent : public Component {
        void init_access() override {}
    };

    ComponentManager& manager = ComponentManager::get_instance();
    Entity id = std::numeric_limits<Entity>::max();
    std::unique_ptr<TestComponent> component1(new TestComponent());
    std::unique_ptr<TestComponent> component2(new TestComponent());
    manager.set_component(id, std::move(component1));
    manager.set_component(id, std::move(component2));

    TestComponent* retrievedComponent = manager.get_component<TestComponent>(id);
    EXPECT_NE(nullptr, retrievedComponent);
    EXPECT_EQ(retrievedComponent, manager.get_component<TestComponent>(id));
}
