#include "world.h"
#include "../engine/physics.h"
#include "../public/log.h"
#include "../public/renderer/irenderer.h"
#include "../renderer/scene.h"

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

void World::AddEntity(std::shared_ptr<Entity> entity)
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
        ent->FixedUpdate(deltaTime);
    }
}

void World::Render()
{
    if (!m_enabled)
    {
        return;
    }

    g_renderer->SetCamera(nullptr);
    for (const auto& ent : m_entities)
    {
        const auto cam = ent->GetComponent<Camera>();
        if (cam && cam->IsActive())
        {
            g_renderer->SetCamera(cam);
            break;
        }
    }

    g_scene->Update(m_entities);
    g_scene->Render();
}
} // namespace yar
