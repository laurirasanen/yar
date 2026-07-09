#include "scene.h"
#include "../public/log.h"
#include "../public/renderer/irenderer.h"
#include "../public/time_util.h"
#include "../renderer/renderer.h"

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
    const auto startTime = Time::Now();
    auto&      stats     = g_renderer->GetRenderStats();

    if (worldNodes.size() <= 0)
    {
        stats.SceneUpdateTime = Time::Now() - startTime;
        return;
    }

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

    CullNodes();
    BatchNodes();
    SortBatches();

    UpdateDescriptor();
}

void Scene::UpdateDescriptor()
{
    const auto startTime = Time::Now();
    auto&      stats     = g_renderer->GetRenderStats();

    // TODO this kinda sucks
    std::vector<std::shared_ptr<IRenderNode>> flattened = {};
    flattened.reserve(m_nodes.size());

    for (const auto& batch : m_batches)
    {
        flattened.append_range(batch.second.Nodes);
    }

    const auto renderer = static_pointer_cast<Renderer>(g_renderer);
    renderer->UpdateDescriptor(flattened);

    stats.SceneDescriptorTime = Time::Now() - startTime;
}

void Scene::Render()
{
    const auto startTime = Time::Now();
    auto&      stats     = g_renderer->GetRenderStats();
    const auto renderer  = static_pointer_cast<Renderer>(g_renderer);

    if (m_nodes.size() <= 0)
    {
        stats.SceneRenderTime = Time::Now() - startTime;
        return;
    }

    uint32_t nodeIdx  = 0;
    uint32_t batchIdx = 0;

    for (const auto& batch : m_batches)
    {
        renderer->BeginDebugLabel(
            std::format("Batch {} ({})", batchIdx, RenderPipelineNames[batch.first]).c_str(),
            {}
        );

        const auto& nodes = batch.second.Nodes;
        stats.NodeCount += nodes.size();

        if (nodes.size() > 0)
        {
            g_renderer->BindPipeline(batch.first);
        }

        for (uint32_t i = 0; i < nodes.size(); i++)
        {
            stats.VertexCount += nodes[i]->GetVertexCount();
            stats.IndexCount += nodes[i]->GetIndexCount();
            g_renderer->BindDescriptor(nodeIdx);
            nodes[i]->Render();
            nodeIdx++;
        }

        batchIdx++;

        renderer->EndDebugLabel();
    }

    stats.SceneRenderTime = Time::Now() - startTime;
}

void Scene::CullNodes()
{
    const auto startTime = Time::Now();
    const auto camera    = g_renderer->GetCamera();
    auto&      stats     = g_renderer->GetRenderStats();

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
            stats.CulledNodeCount++;
            stats.CulledVertexCount += node->GetVertexCount();
            stats.CulledIndexCount += node->GetIndexCount();
        }
        else
        {
            visible.push_back(node);
        }
    }
    m_nodes = visible;

    stats.SceneCullTime = Time::Now() - startTime;
}

void Scene::BatchNodes()
{
    const auto startTime = Time::Now();
    auto&      stats     = g_renderer->GetRenderStats();

    for (auto& batch : m_batches)
    {
        batch.second.Nodes.clear();
    }

    for (const auto& node : m_nodes)
    {
        m_batches[node->GetPipeline()].Nodes.push_back(node);
    }

    stats.SceneBatchTime = Time::Now() - startTime;
}

void Scene::SortBatches()
{
    const auto startTime = Time::Now();
    const auto camera    = g_renderer->GetCamera();
    const auto cameraPos = camera->transform.GetPosition();

    for (auto& batch : m_batches)
    {
        std::sort(
            batch.second.Nodes.begin(),
            batch.second.Nodes.end(),
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
    }

    auto& stats         = g_renderer->GetRenderStats();
    stats.SceneSortTime = Time::Now() - startTime;
}
} // namespace yar
