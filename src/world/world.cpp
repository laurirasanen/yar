#include "world.h"
#include "../components/transform.h"
#include "../log.h"

namespace yar
{

World::World(std::shared_ptr<Renderer> renderer, std::shared_ptr<UI> ui) :
    m_renderer(renderer),
    m_ui(ui)
{
    LOG_INFO("Creating World");

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

    m_scenes.push_back(std::make_shared<Scene>(renderer, ui, "assets/scenes/bistro.glb"));
    m_scenes[0]->GetTransform().SetScale({0.01, 0.01, 0.01});
    m_scenes[0]->UpdateAABB();

    m_scenes.push_back(std::make_shared<Scene>(renderer, ui, "assets/scenes/DamagedHelmet.glb"));
    m_scenes[1]->GetTransform().SetEulerRotation({180, 0, -90});
    m_scenes[1]->GetTransform().SetScale({0.25, 0.25, 0.25});
    m_scenes[1]->GetTransform().SetPosition({-0.4, -3.3, 1.3});
    m_scenes[1]->UpdateAABB();
}

World::~World()
{
    LOG_INFO("Destroying World");
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
    Transform trans = {};
    m_renderer->SetModelMatrix(trans);
    m_renderer->DrawWithBuffers(m_testPlaneVertexBuffer, m_testPlaneIndexBuffer);

    for (const auto& scene : m_scenes)
    {
        scene->FrustumCull(camera);
        scene->Render(m_renderer);
    }
}
} // namespace yar
