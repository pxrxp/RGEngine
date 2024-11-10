#include <gtest/gtest.h>
#include <Core/EntityManager.h>
#include <Core/ServiceLocator.h>

class EntityManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        EntityManager& manager = ServiceLocator::get_entity_manager();
        manager.reset();
    }
};

TEST_F(EntityManagerTest, SingletonTest) {
    EntityManager& instance1 = ServiceLocator::get_entity_manager();
    EntityManager& instance2 = ServiceLocator::get_entity_manager();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(EntityManagerTest, CreateEntityTest) {
    EntityManager& manager = ServiceLocator::get_entity_manager();
    Entity entity = manager.create_entity();

    EXPECT_EQ(entity, 1);
}

TEST_F(EntityManagerTest, CreateMultipleEntitiesTest) {
    EntityManager& manager = ServiceLocator::get_entity_manager();
    std::pair<Entity, Entity> entities = manager.create_entities(5);

    EXPECT_EQ(entities.first, 1);
    EXPECT_EQ(entities.second, 5);
}

TEST_F(EntityManagerTest, CreateEntitiesZeroCountTest) {
    EntityManager& manager = ServiceLocator::get_entity_manager();
    std::pair<Entity, Entity> entities = manager.create_entities(0);

    EXPECT_EQ(entities.first, 0);
    EXPECT_EQ(entities.second, 0);
}

TEST_F(EntityManagerTest, CreateEntitiesCountExceedingMaxTest) {
    EntityManager& manager = ServiceLocator::get_entity_manager();
    std::pair<Entity, Entity> entities = manager.create_entities(MAX_ENTITIES + 1);

    EXPECT_EQ(entities.first, 1);
    EXPECT_EQ(entities.second, MAX_ENTITIES);
}

TEST_F(EntityManagerTest, CreateEntitiesMultipleTimesTest) {
    EntityManager& manager = ServiceLocator::get_entity_manager();
    std::pair<Entity, Entity> entities1 = manager.create_entities(5);
    std::pair<Entity, Entity> entities2 = manager.create_entities(3);

    EXPECT_EQ(entities1.first, 1);
    EXPECT_EQ(entities1.second, 5);
    EXPECT_EQ(entities2.first, 6);
    EXPECT_EQ(entities2.second, 8);
}

TEST_F(EntityManagerTest, CreateEntitiesExceedingMaxTest) {
    EntityManager& manager = ServiceLocator::get_entity_manager();
    for (int i = 0; i < MAX_ENTITIES; ++i) {
        manager.create_entity();
    }

    Entity entity = manager.create_entity();
    EXPECT_EQ(entity, 0);
}

