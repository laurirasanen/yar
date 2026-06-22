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

    uint8_t pixel[]       = {255, 0, 255, 255};
    auto    missingAlbedo = std::make_shared<Texture>(
        m_renderer,
        "_YAR_MISSING_ALBEDO",
        4,
        pixel,
        TextureFormat::FMT_SRGB,
        true
    );

    pixel[0]           = 128;
    pixel[1]           = 128;
    pixel[2]           = 255;
    auto missingNormal = std::make_shared<Texture>(
        m_renderer,
        "_YAR_MISSING_NORMAL",
        4,
        pixel,
        TextureFormat::FMT_SRGB,
        true
    );

    pixel[0]         = 0;
    pixel[1]         = 200;
    pixel[2]         = 0;
    auto missingMRAO = std::make_shared<Texture>(
        m_renderer,
        "_YAR_MISSING_MRAO",
        4,
        pixel,
        TextureFormat::FMT_SRGB,
        true
    );

    m_renderer->SetMissingTexture(TextureType::TEX_ALBEDO, missingAlbedo);
    m_renderer->SetMissingTexture(TextureType::TEX_ALBEDO, missingNormal);
    m_renderer->SetMissingTexture(TextureType::TEX_MRAO, missingMRAO);
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

    auto      bistro = std::make_shared<Scene>(m_renderer, m_ui, "assets/scenes/bistro.glb");
    Transform trans  = {};
    trans.SetEulerRotation({90, 0, 0});
    trans.SetScale({0.01, 0.01, 0.01});
    bistro->SetTransform(trans);
    m_scenes.push_back(bistro);

    auto      helmet = std::make_shared<Scene>(m_renderer, m_ui, "assets/scenes/DamagedHelmet.glb");
    Transform helmTrans = {};
    helmTrans.SetEulerRotation({180, 0, -90});
    helmTrans.SetScale({0.25, 0.25, 0.25});
    helmTrans.SetPosition({-0.4, -3.3, 1.3});
    helmet->SetTransform(helmTrans);
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
