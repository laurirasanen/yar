#include "world.h"
#include "../platform/fs.h"
#include "../public/irenderer.h"
#include "../public/log.h"
#include "../public/transform.h"
#include "../public/util.h"

namespace yar
{
World::World() : IWorld()
{
    LOG_INFO("Creating World");
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
}

void World::Render()
{
    if (!m_enabled)
    {
        return;
    }

    if (m_nodes.size() > 0)
    {
        g_scene->SetNodes(m_nodes);
        g_scene->UpdateDescriptor();
        g_scene->Render();
    }

    if (m_sky)
    {
        m_sky->Render();
    }
}
} // namespace yar
