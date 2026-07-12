#include "world.h"
#include "../engine/physics.h"
#include "../public/log.h"
#include "../public/renderer/irenderer.h"
#include "../renderer/scene.h"

namespace yar
{
std::shared_ptr<IPhysics> g_physics;

World::World() : IWorld(), m_sky(nullptr)
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

    auto sky = entity->GetComponent<SkyComponent>();
    if (sky)
    {
        m_sky = sky;
        g_renderer->SetSky(sky);
    }
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
        // TODO this is silly
        const auto cam = ent->GetComponent<NoclipCamera>();
        if (cam && cam->IsActive())
        {
            g_renderer->SetCamera(cam);
            break;
        }

        const auto cam2 = ent->GetComponent<Camera>();
        if (cam2 && cam2->IsActive())
        {
            g_renderer->SetCamera(cam2);
            break;
        }
    }

    g_scene->Update(m_entities);
    g_scene->SetSky(m_sky);
    g_scene->Render();
}
} // namespace yar
