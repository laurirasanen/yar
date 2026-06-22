#include "world.h"
#include "../components/transform.h"
#include "../log.h"

namespace yar
{
World::World(std::shared_ptr<Renderer> renderer, std::shared_ptr<UI> ui) :
    m_renderer(renderer),
    m_ui(ui),
    m_loaded(false)
{
    LOG_INFO("Creating World");
}

World::~World()
{
    LOG_INFO("Destroying World");
}

void World::Load()
{
    LOG_INFO("Loading World");

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
    m_renderer->CreateBuffer(
        m_skyVertexBuffer,
        VertexBuffer,
        skyVertices.data(),
        sizeof(VertexUnlit),
        static_cast<uint32_t>(skyVertices.size())
    );
    m_renderer->CreateBuffer(
        m_skyIndexBuffer,
        IndexBuffer,
        skyIndices.data(),
        sizeof(Index),
        static_cast<uint32_t>(skyIndices.size())
    );

    Transform trans = {};

    auto bistro = std::make_shared<Scene>(m_renderer, m_ui, "assets/scenes/bistro.glb");
    trans.SetScale({0.01, 0.01, 0.01});
    bistro->SetTransform(trans);
    m_scenes.push_back(bistro);

    auto helmet = std::make_shared<Scene>(m_renderer, m_ui, "assets/scenes/DamagedHelmet.glb");
    trans.SetEulerRotation({180, 0, -90});
    trans.SetScale({0.25, 0.25, 0.25});
    trans.SetPosition({-0.4, -3.3, 1.3});
    helmet->SetTransform(trans);
    m_scenes.push_back(helmet);
}

void World::Frame()
{
}

void World::Tick()
{
    std::scoped_lock worldLock {m_worldMutex};

    if (!m_loaded)
    {
        m_ui->ToggleWindow(UIWindow::LOADING);
        Load();
        m_ui->ToggleWindow(UIWindow::LOADING);
        m_loaded = true;
    }
}

void World::Render(std::shared_ptr<Camera> camera)
{
    if (!m_loaded)
    {
        return;
    }

    m_renderer->BindPipeline(RenderPipeline::SHADED);

    std::vector<std::shared_ptr<Mesh<VertexShaded>>> meshes = {};

    for (const auto& scene : m_scenes)
    {
        meshes.append_range(scene->GetMeshes());
    }

    m_renderer->CullMeshes(camera, meshes);
    m_renderer->SortMeshes(camera, meshes);
    m_renderer->UpdateDescriptor(meshes);

    for (uint32_t i = 0; i < meshes.size(); i++)
    {
        const auto& mesh = meshes[i];
        m_renderer->BindDescriptor(i);
        m_renderer->DrawWithBuffers(mesh->GetVertexBuffer(), mesh->GetIndexBuffer());
    }

    m_renderer->BindPipeline(RenderPipeline::UNLIT);
    m_renderer->BindDescriptor(0);
    m_renderer->DrawWithBuffers(m_skyVertexBuffer, m_skyIndexBuffer);
}
} // namespace yar
