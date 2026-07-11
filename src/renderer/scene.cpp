#include "scene.h"
#include "../public/ecs/entity.h"
#include "../public/ecs/mesh.h"
#include "../public/renderer/irenderer.h"
#include "../public/resource/mesh.h"
#include "../public/time_util.h"
#include "../renderer/renderer.h"
#include "src/public/ecs/transform.h"

namespace yar
{
Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::Update(const std::vector<std::shared_ptr<Entity>>& entities)
{
    const auto startTime = Time::Now();
    auto&      stats     = g_renderer->GetRenderStats();

    m_nodes.clear();

    for (const auto& ent : entities)
    {
        const auto mesh  = ent->GetComponent<MeshComponent>();
        const auto trans = ent->GetComponent<TransformComponent>();
        if (!mesh || !trans)
        {
            continue;
        }
        const auto t = *trans->GetTransform();

        const auto subMeshes = mesh->GetMesh()->GetSubMeshes();
        for (const auto& m : subMeshes)
        {
            auto node = std::make_shared<Node>();
            node->SetIndexBuffer(m.GetIndexBuffer());
            node->SetVertices(m.GetVertices());
            node->SetAABB(m.GetAABB());
            node->SetMaterial(m.GetMaterial());
            node->SetTransform(t);
            m_nodes.push_back(node);
        }
    }

    stats.SceneUpdateTime = Time::Now() - startTime;

    CullNodes();
    BatchNodes();
    SortBatches();

    UpdateDescriptor();
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
        renderer->BeginDebugLabel(std::format("Batch {}", batchIdx).c_str(), {});

        const auto& nodes = batch.second.Nodes;

        if (nodes.size() <= 0)
        {
            continue;
        }

        stats.NodeCount += nodes.size();

        const auto& pipe = renderer->GetPipeline(nodes[0]->GetMaterial());
        g_renderer->BindPipeline(pipe->GetVkPipeline(), pipe->GetVkPipelineLayout());

        for (uint32_t i = 0; i < nodes.size(); i++)
        {
            stats.IndexCount += nodes[i]->GetIndexCount();
            g_renderer->BindDescriptor(nodeIdx, pipe->GetVkPipelineLayout());
            renderer->DrawWithBuffers(nodes[i]->GetIndexBuffer());
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
    auto&      stats     = g_renderer->GetRenderStats();
    const auto camera    = g_renderer->GetCamera();

    if (camera == nullptr)
    {
        m_nodes.clear();
        stats.SceneCullTime = Time::Now() - startTime;
        return;
    }

    std::vector<std::shared_ptr<Node>> visible;
    for (auto& node : m_nodes)
    {
        if (node->FrustumCull(camera))
        {
            stats.CulledNodeCount++;
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
        const auto& id = node->GetMaterial().GetShader()->GetId();
        m_batches[id].Nodes.push_back(node);
    }

    stats.SceneBatchTime = Time::Now() - startTime;
}

void Scene::SortBatches()
{
    const auto startTime = Time::Now();
    auto&      stats     = g_renderer->GetRenderStats();
    const auto camera    = g_renderer->GetCamera();
    if (camera == nullptr)
    {
        stats.SceneSortTime = Time::Now() - startTime;
        return;
    }
    const auto cameraPos = camera->transform.GetPosition();

    for (auto& batch : m_batches)
    {
        std::sort(
            batch.second.Nodes.begin(),
            batch.second.Nodes.end(),
            [&cameraPos](std::shared_ptr<Node> a, std::shared_ptr<Node> b) {
                /*
                const auto queueA = a->GetMaterial()->GetQueue();
                const auto queueB = b->GetMaterial()->GetQueue();

                if (queueA < queueB)
                {
                    return true;
                }

                if (queueA == queueB)
                */
                {
                    const auto distA = glm::length(cameraPos - a->GetAABB().center);
                    const auto distB = glm::length(cameraPos - b->GetAABB().center);
                    return distA < distB;
                }

                // return false;
            }
        );
    }

    stats.SceneSortTime = Time::Now() - startTime;
}

void Scene::UpdateDescriptor()
{
    const auto startTime = Time::Now();
    auto&      stats     = g_renderer->GetRenderStats();

    // TODO this kinda sucks
    std::vector<std::shared_ptr<Node>> flattened = {};
    flattened.reserve(m_nodes.size());

    for (const auto& batch : m_batches)
    {
        flattened.append_range(batch.second.Nodes);
    }

    const auto renderer = static_pointer_cast<Renderer>(g_renderer);
    renderer->UpdateDescriptor(flattened);

    stats.SceneDescriptorTime = Time::Now() - startTime;
}
} // namespace yar
