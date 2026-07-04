#include "world.h"
#include "../engine/physics.h"
#include "../platform/fs.h"
#include "../public/irenderer.h"
#include "../public/log.h"
#include "../public/transform.h"
#include "../public/util.h"

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

void World::AddNode(std::shared_ptr<INode> node)
{
    m_nodes.push_back(node);
}

void World::Frame()
{
}

void World::Tick()
{
    std::scoped_lock worldLock {m_worldMutex};

    for (const auto& node : m_nodes)
    {
        node->EarlyTick();
    }

    g_physics->Step();

    for (const auto& node : m_nodes)
    {
        node->Tick();
    }
}

void World::Render()
{
    if (!m_enabled)
    {
        return;
    }

    g_scene->Update(m_nodes);
    g_scene->Render();

    if (m_sky)
    {
        m_sky->Render();
    }
}
} // namespace yar
