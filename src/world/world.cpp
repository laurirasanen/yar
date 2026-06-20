#include "world.h"
#include "../log.h"

namespace yar
{

World::World(std::shared_ptr<Renderer> renderer) : m_renderer(renderer)
{
    LOG_DEBUG("Creating World");

    // test plane
    std::vector<VertexUnlit> planeVertices = {
        {.position = {-5.0, -5.0, -2.0}, .color = {1.0f, 0.0f, 0.0f}},
        { .position = {5.0, -5.0, -2.0}, .color = {0.0f, 1.0f, 0.0f}},
        {  .position = {5.0, 5.0, -2.0}, .color = {0.0f, 0.0f, 1.0f}},
        { .position = {-5.0, 5.0, -2.0}, .color = {1.0f, 1.0f, 1.0f}},
    };
    std::vector<Index> planeIndices = {0, 1, 2, 2, 3, 0};
    renderer->CreateBuffer(
        m_testPlaneVertexBuffer,
        VertexBuffer,
        planeVertices.data(),
        sizeof(VertexUnlit),
        static_cast<uint32_t>(planeVertices.size())
    );
    renderer->CreateBuffer(
        m_testPlaneIndexBuffer,
        IndexBuffer,
        planeIndices.data(),
        sizeof(Index),
        static_cast<uint32_t>(planeIndices.size())
    );

    // test model
    m_models.push_back(std::make_shared<Model>(renderer, "assets/models/DamagedHelmet.glb"));
    m_models[0]->GetTransform().SetEulerRotation({180, 0, 0});
}

World::~World()
{
    LOG_DEBUG("Destroying World");
}

void World::Frame()
{
}

void World::Tick()
{
    {
        std::scoped_lock worldLock {m_worldMutex};
    }
}

void World::Render(std::shared_ptr<Camera> camera)
{
    m_renderer->BindPipeline(RenderPipeline::UNLIT);
    m_renderer->DrawWithBuffers(m_testPlaneVertexBuffer, m_testPlaneIndexBuffer);

    for (const auto& model : m_models)
    {
        if (model->FrustumCull(camera))
        {
            model->MarkAsCulled(m_renderer);
            continue;
        }
        model->Render(m_renderer);
    }
}
} // namespace yar
