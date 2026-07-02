#include "scene.h"
#include "../public/irenderer.h"
#include "../public/log.h"
#include "../renderer/renderer.h"

namespace yar
{
Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::SetNodes(std::vector<std::shared_ptr<INode>> nodes)
{
    m_nodes.clear();

    for (const auto& node : nodes)
    {
        m_nodes.append_range(node->GetChildrenRecursive());
    }

    CullNodes();
    SortNodes();
}

void Scene::UpdateDescriptor()
{
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
        g_renderer->BindPipeline(RenderPipeline::SHADED); // TODO
        g_renderer->BindDescriptor(i);
        m_nodes[i]->Render();
    }
}

void Scene::CullNodes()
{
    const auto startTime = Time::Now();

    const auto camera = g_renderer->GetCamera();
    auto       stats  = g_renderer->GetCullStats();

    std::vector<std::shared_ptr<INode>> visible;
    for (const auto& node : m_nodes)
    {
        if (!node->Renderable())
        {
            continue;
        }

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
        [&cameraPos](std::shared_ptr<INode> a, std::shared_ptr<INode> b) {
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
