#include "sky.h"
#include "../platform/fs.h"
#include "../public/iui.h"
#include "../renderer/data_types.h"
#include "../renderer/renderer.h"

namespace yar
{
Sky::Sky(std::string folder) : ISky()
{
    const auto f = fs_relative_path(folder);
    g_ui->SetLoadingText(f.c_str());

    const auto iblColorPath    = fs_append(f, "color.hdr");
    const auto iblLUTPath      = fs_append(f, "lut.png");
    const auto iblDiffusePath  = fs_append(f, "diffuse.ktx2");
    const auto iblSpecularPath = fs_append(f, "specular.ktx2");

    if (!fs_exists(iblColorPath) || !fs_exists(iblLUTPath) || !fs_exists(iblDiffusePath)
        || !fs_exists(iblSpecularPath))
    {
        throw std::runtime_error("Missing IBL texture");
    }

    const auto renderer = static_pointer_cast<Renderer>(g_renderer);

    const auto colorData = fs_read_data(iblColorPath);
    m_color              = std::make_shared<Texture>(
        renderer,
        "_YAR_IBL_COLOR",
        colorData.size(),
        colorData.data(),
        TextureType::TEX_IBL,
        false
    );

    const auto lutData = fs_read_data(iblLUTPath);
    m_lut              = std::make_shared<Texture>(
        renderer,
        "_YAR_IBL_LUT",
        lutData.size(),
        lutData.data(),
        TextureType::TEX_IBL_LUT,
        false
    );

    const auto diffuseData = fs_read_data(iblDiffusePath);
    m_diffuse              = std::make_shared<Texture>(
        renderer,
        "_YAR_IBL_DIFFUSE",
        diffuseData.size(),
        diffuseData.data(),
        TextureType::TEX_KTX,
        false
    );

    const auto specularData = fs_read_data(iblSpecularPath);
    m_specular              = std::make_shared<Texture>(
        renderer,
        "_YAR_IBL_SPECULAR",
        specularData.size(),
        specularData.data(),
        TextureType::TEX_KTX,
        false
    );

    std::vector<VertexSky> vertices = {
        {.position = {-1.0f, -1.0f}},
        {.position = {3.0f, -1.0f}},
        {.position = {-1.0f, 3.0f}},
    };
    std::vector<Index> indices = {2, 1, 0};
    g_renderer->CreateBuffer(
        m_vertexBuffer,
        VertexBuffer,
        vertices.data(),
        sizeof(VertexSky),
        static_cast<uint32_t>(vertices.size())
    );
    g_renderer->CreateBuffer(
        m_indexBuffer,
        IndexBuffer,
        indices.data(),
        sizeof(Index),
        static_cast<uint32_t>(indices.size())
    );
}

void Sky::Render()
{
    g_renderer->BindPipeline(RenderPipeline::SKY);
    g_renderer->BindDescriptor(0);
    g_renderer->DrawWithBuffers(m_vertexBuffer, m_indexBuffer);
}
}; // namespace yar
