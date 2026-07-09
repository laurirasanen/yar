#include "world.h"
#include "../engine/physics.h"
#include "../public/log.h"

namespace yar
{
std::shared_ptr<IPhysics> g_physics;

World::World() : IWorld()
{
    LOG_INFO("Creating World");
    g_physics = std::make_shared<Physics>();
}

World::~World()
{
    LOG_INFO("Destroying World");
}

void World::AddNode(std::shared_ptr<Entity> entity)
{
    entity->Initialize();
    m_entities.push_back(entity);
}

void World::Update(float deltaTime)
{
    for (const auto& ent : m_entities)
    {
        ent->Update(deltaTime);
    }
}

void World::FixedUpdate(float deltaTime)
{
    std::scoped_lock worldLock {m_worldMutex};

    for (const auto& ent : m_entities)
    {
        ent->EarlyFixedUpdate(deltaTime);
    }

    g_physics->Step(deltaTime);

    for (const auto& ent : m_entities)
    {
        ent->FixedUpate(deltaTime);
    }
}

void World::Render()
{
    if (!m_enabled)
    {
        return;
    }

    g_scene->Update(m_entities);
    g_scene->Render();
}
} // namespace yar
