#include "scene.h"
#include "../public/irenderer.h"
#include "../public/log.h"
#include "../public/time_util.h"
#include "../renderer/renderer.h"
#include "../world/mesh_node.h"

namespace yar
{
Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::Update(std::vector<std::shared_ptr<INode>> worldNodes)
{
    auto& stats = g_renderer->GetRenderStats();
    if (worldNodes.size() <= 0)
    {
        stats.SceneUpdateTime = 0;
        return;
    }

    const auto startTime = Time::Now();

    std::vector<std::shared_ptr<INode>> flattened = {};
    for (const auto& node : worldNodes)
    {
        flattened.append_range(node->GetChildrenRecursive());
    }
    flattened.append_range(worldNodes);

    m_nodes.clear();
    for (const auto& node : flattened)
    {
        const auto& r = dynamic_pointer_cast<IRenderNode>(node);
        if (r != nullptr)
        {
            m_nodes.push_back(r);
        }
    }

    stats.SceneUpdateTime = Time::Now() - startTime;

    // TODO: batching

    CullNodes();
    SortNodes();
}

void Scene::UpdateDescriptor()
{
    if (m_nodes.size() <= 0)
    {
        return;
    }

    const auto renderer = static_pointer_cast<Renderer>(g_renderer);
    renderer->UpdateDescriptor(m_nodes);
}

void Scene::Render()
{
    if (m_nodes.size() <= 0)
    {
        return;
    }

    auto& stats     = g_renderer->GetRenderStats();
    stats.MeshCount = m_nodes.size();
    for (uint32_t i = 0; i < m_nodes.size(); i++)
    {
        stats.VertexCount += m_nodes[i]->GetVertexCount();
        stats.IndexCount += m_nodes[i]->GetIndexCount();
        // TODO this sucks
        g_renderer->BindPipeline(m_nodes[i]->GetPipeline());
        g_renderer->BindDescriptor(i);
        m_nodes[i]->Render();
    }
}

void Scene::CullNodes()
{
    const auto startTime = Time::Now();

    const auto camera = g_renderer->GetCamera();
    auto&      stats  = g_renderer->GetCullStats();

    std::vector<std::shared_ptr<IRenderNode>> visible;
    for (const auto& node : m_nodes)
    {
        if (node->GetMaterial() == nullptr)
        {
            LOG_ERROR("Node is missing a material: {}", node->GetName());
            continue;
        }

        if (node->FrustumCull(camera))
        {
            stats.MeshCount++;
            stats.VertexCount += node->GetVertexCount();
            stats.IndexCount += node->GetIndexCount();
        }
        else
        {
            visible.push_back(node);
        }
    }
    m_nodes = visible;

    stats.CullTime = Time::Now() - startTime;
}

void Scene::SortNodes()
{
    const auto startTime = Time::Now();

    const auto camera    = g_renderer->GetCamera();
    const auto cameraPos = camera->transform.GetPosition();

    std::sort(
        m_nodes.begin(),
        m_nodes.end(),
        [&cameraPos](std::shared_ptr<IRenderNode> a, std::shared_ptr<IRenderNode> b) {
            const auto queueA = a->GetMaterial()->GetQueue();
            const auto queueB = b->GetMaterial()->GetQueue();

            if (queueA < queueB)
            {
                return true;
            }

            if (queueA == queueB)
            {
                const auto distA = glm::length(cameraPos - a->GetAABB().center);
                const auto distB = glm::length(cameraPos - b->GetAABB().center);
                return distA < distB;
            }

            return false;
        }
    );

    auto& stats    = g_renderer->GetRenderStats();
    stats.SortTime = Time::Now() - startTime;
}
} // namespace yar
