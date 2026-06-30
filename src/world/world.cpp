#include "world.h"
#include "../components/transform.h"
#include "../log.h"
#include "../platform/fs.h"
#include "../util.h"

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

    m_ui->SetLoadingScene("Global textures");

    const char* iblColorPath    = "assets/ibl/cobble/color.hdr";
    const char* iblLUTPath      = "assets/ibl/cobble/lut.png";
    const char* iblDiffusePath  = "assets/ibl/cobble/diffuse.ktx2";
    const char* iblSpecularPath = "assets/ibl/cobble/specular.ktx2";
    if (!fs_exists(iblColorPath) || !fs_exists(iblLUTPath) || !fs_exists(iblDiffusePath)
        || !fs_exists(iblSpecularPath))
    {
        throw std::runtime_error("Missing IBL texture");
    }

    m_ui->SetLoadingTexture(iblColorPath);
    const auto iblColorData = fs_read_data(iblColorPath);
    auto       iblColor     = std::make_shared<Texture>(
        m_renderer,
        "_YAR_IBL_COLOR",
        iblColorData.size(),
        iblColorData.data(),
        TextureType::TEX_IBL,
        false
    );

    m_ui->SetLoadingTexture(iblLUTPath);
    const auto iblLUTData = fs_read_data(iblLUTPath);
    auto       iblLUT     = std::make_shared<Texture>(
        m_renderer,
        "_YAR_IBL_LUT",
        iblLUTData.size(),
        iblLUTData.data(),
        TextureType::TEX_IBL_LUT,
        false
    );

    m_ui->SetLoadingTexture(iblDiffusePath);
    const auto iblDiffuseData = fs_read_data(iblDiffusePath);
    auto       iblDiffuse     = std::make_shared<Texture>(
        m_renderer,
        "_YAR_IBL_DIFFUSE",
        iblDiffuseData.size(),
        iblDiffuseData.data(),
        TextureType::TEX_KTX,
        false
    );

    m_ui->SetLoadingTexture(iblSpecularPath);
    const auto iblSpecularData = fs_read_data(iblSpecularPath);
    auto       iblSpecular     = std::make_shared<Texture>(
        m_renderer,
        "_YAR_IBL_SPECULAR",
        iblSpecularData.size(),
        iblSpecularData.data(),
        TextureType::TEX_KTX,
        false
    );

    m_renderer->SetIBL(iblColor, iblLUT, iblDiffuse, iblSpecular);

    m_ui->SetLoadingTexture("_YAR_MISSING_ALBEDO");
    uint8_t pixel[]       = {255, 0, 255, 0};
    auto    missingAlbedo = std::make_shared<Texture>(
        m_renderer,
        "_YAR_MISSING_ALBEDO",
        4,
        pixel,
        TextureType::TEX_ALBEDO,
        true
    );

    m_ui->SetLoadingTexture("_YAR_MISSING_ORM");
    pixel[0]         = 0;
    pixel[1]         = 200;
    pixel[2]         = 0;
    auto missingMRAO = std::make_shared<Texture>(
        m_renderer,
        "_YAR_MISSING_ORM",
        4,
        pixel,
        TextureType::TEX_ORM,
        true
    );

    m_ui->SetLoadingTexture("_YAR_MISSING_NORMAL");
    pixel[0]           = 128;
    pixel[1]           = 128;
    pixel[2]           = 255;
    auto missingNormal = std::make_shared<Texture>(
        m_renderer,
        "_YAR_MISSING_NORMAL",
        4,
        pixel,
        TextureType::TEX_NORMAL,
        true
    );

    m_ui->SetLoadingTexture("_YAR_MISSING_EMISSIVE");
    pixel[0]             = 0;
    pixel[1]             = 0;
    pixel[2]             = 0;
    pixel[3]             = 0;
    auto missingEmissive = std::make_shared<Texture>(
        m_renderer,
        "_YAR_MISSING_EMISSIVE",
        4,
        pixel,
        TextureType::TEX_EMISSIVE,
        true
    );

    m_ui->SetLoadingTexture("");

    m_renderer->SetMissingTexture(TextureType::TEX_ALBEDO, missingAlbedo);
    m_renderer->SetMissingTexture(TextureType::TEX_ORM, missingMRAO);
    m_renderer->SetMissingTexture(TextureType::TEX_NORMAL, missingNormal);
    m_renderer->SetMissingTexture(TextureType::TEX_EMISSIVE, missingEmissive);

    m_ui->SetLoadingScene("Sky");

    // clang-format off
    std::vector<VertexSky> skyVertices = {
        {.position = {-1.0f, -1.0f}},
        {.position = {3.0f, -1.0f}},
        {.position = {-1.0f, 3.0f}},
    };
    std::vector<Index> skyIndices = {
        2, 1, 0
    };
    // clang-format on
    m_renderer->CreateBuffer(
        m_skyVertexBuffer,
        VertexBuffer,
        skyVertices.data(),
        sizeof(VertexSky),
        static_cast<uint32_t>(skyVertices.size())
    );
    m_renderer->CreateBuffer(
        m_skyIndexBuffer,
        IndexBuffer,
        skyIndices.data(),
        sizeof(Index),
        static_cast<uint32_t>(skyIndices.size())
    );

    // Note: these are purposefully not instanced to simulate loading a bunch of unique meshes.
    for (uint32_t x = 0; x < 1; x++)
    {
        for (uint32_t z = 0; z < 1; z++)
        {
            Transform trans = {};

            auto flightHelmet =
                std::make_shared<Scene>(m_renderer, m_ui, "assets/scenes/FlightHelmet.glb");
            trans.SetEulerRotation({0, 0, 0});
            trans.SetPosition({-0.4 + x * -0.75, -0.3, z * 0.75});
            flightHelmet->SetTransform(trans);
            m_scenes.push_back(flightHelmet);

            auto damagedHelmet =
                std::make_shared<Scene>(m_renderer, m_ui, "assets/scenes/DamagedHelmet.glb");
            trans = {};
            trans.SetEulerRotation({90, 0, 0});
            trans.SetScale({0.3, 0.3, 0.3});
            trans.SetPosition({0.4 + x * 0.75, 0, z * 0.75});
            damagedHelmet->SetTransform(trans);
            m_scenes.push_back(damagedHelmet);
        }
    }
}

void World::Frame()
{
    if (!m_loaded)
    {
        return;
    }

    const float rotateSpeeds[] = {12.5f, -12.5f, 25.0f, -25.0f, 50.0f};
    for (size_t i = 0; i < m_scenes.size(); i++)
    {
        const auto& scene = m_scenes[i];
        for (auto& mesh : scene->GetMeshes())
        {
            const float delta =
                static_cast<float>(Time::DeltaFrame) * rotateSpeeds[i % ARRAY_SIZE(rotateSpeeds)];
            mesh->GetTransform()->AddRotation(delta, VEC_UP);
            mesh->UpdateAABB();
        }
    }
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

    m_renderer->BindPipeline(RenderPipeline::SKY);
    m_renderer->BindDescriptor(0);
    m_renderer->DrawWithBuffers(m_skyVertexBuffer, m_skyIndexBuffer);
}
} // namespace yar
