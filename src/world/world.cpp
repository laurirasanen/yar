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

    // clang-format off
    const float skyExtent = 100.0f;
    std::vector<VertexUnlit> skyVertices = {
        // bottom
        {.position = {-skyExtent, -skyExtent, -skyExtent}, .color = {0.15f, 0.5f, 1.0f}},
        {.position = { skyExtent, -skyExtent, -skyExtent}, .color = {0.15f, 0.5f, 1.0f}},
        {.position = { skyExtent,  skyExtent, -skyExtent}, .color = {0.15f, 0.5f, 1.0f}},
        {.position = {-skyExtent,  skyExtent, -skyExtent}, .color = {0.15f, 0.5f, 1.0f}},
        // top
        {.position = {-skyExtent, -skyExtent, skyExtent}, .color = {0.15f, 0.5f, 1.0f}},
        {.position = { skyExtent, -skyExtent, skyExtent}, .color = {0.15f, 0.5f, 1.0f}},
        {.position = { skyExtent,  skyExtent, skyExtent}, .color = {0.15f, 0.5f, 1.0f}},
        {.position = {-skyExtent,  skyExtent, skyExtent}, .color = {0.15f, 0.5f, 1.0f}},
    };
    std::vector<Index> skyIndices = {
        // bottom (-Z)
        0, 1, 2, 2, 3, 0,
        // top (+Z)
        6, 5, 4, 4, 7, 6,
        // left (-X)
        0, 3, 4, 4, 3, 7,
        // right (+X)
        5, 2, 1, 6, 2, 5,
        // front (+Y)
        7, 3, 2, 2, 6, 7,
        // back (-Y)
        1, 0, 4, 4, 5, 1,
    };
    // clang-format on
    renderer->CreateBuffer(
        m_skyVertexBuffer,
        VertexBuffer,
        skyVertices.data(),
        sizeof(VertexUnlit),
        static_cast<uint32_t>(skyVertices.size())
    );
    renderer->CreateBuffer(
        m_skyIndexBuffer,
        IndexBuffer,
        skyIndices.data(),
        sizeof(Index),
        static_cast<uint32_t>(skyIndices.size())
    );

    auto bistro = std::make_shared<Scene>(renderer, ui, "assets/scenes/bistro.glb");
    bistro->GetTransform().SetScale({0.01, 0.01, 0.01});
    bistro->UpdateAABB();
    m_scenes.push_back(bistro);

    auto helmet = std::make_shared<Scene>(renderer, ui, "assets/scenes/DamagedHelmet.glb");
    helmet->GetTransform().SetEulerRotation({180, 0, -90});
    helmet->GetTransform().SetScale({0.25, 0.25, 0.25});
    helmet->GetTransform().SetPosition({-0.4, -3.3, 1.3});
    helmet->UpdateAABB();
    m_scenes.push_back(helmet);
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
    m_renderer->DrawWithBuffers(m_skyVertexBuffer, m_skyIndexBuffer);

    for (const auto& scene : m_scenes)
    {
        scene->FrustumCull(camera);
        scene->Render(m_renderer);
    }
}
} // namespace yar
