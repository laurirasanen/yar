#include "assets.h"

#include "../renderer/renderer.h"
#include "../renderer/texture.h"
#include "../world/gltf_node.h"
#include "../world/mesh_node.h"
#include "../world/sky.h"
#include "src/public/irenderer.h"

#include <memory>

namespace yar
{
void Assets::Initialize()
{
    auto renderer = static_pointer_cast<Renderer>(g_renderer);

    uint8_t pixel[]       = {255, 0, 255, 0};
    auto    missingAlbedo = std::make_shared<Texture>(
        renderer,
        "_YAR_MISSING_ALBEDO",
        4,
        pixel,
        TextureType::TEX_ALBEDO,
        true
    );

    pixel[0]         = 0;
    pixel[1]         = 200;
    pixel[2]         = 0;
    auto missingMRAO = std::make_shared<Texture>(
        renderer,
        "_YAR_MISSING_ORM",
        4,
        pixel,
        TextureType::TEX_ORM,
        true
    );

    pixel[0]           = 128;
    pixel[1]           = 128;
    pixel[2]           = 255;
    auto missingNormal = std::make_shared<Texture>(
        renderer,
        "_YAR_MISSING_NORMAL",
        4,
        pixel,
        TextureType::TEX_NORMAL,
        true
    );

    pixel[0]             = 0;
    pixel[1]             = 0;
    pixel[2]             = 0;
    pixel[3]             = 0;
    auto missingEmissive = std::make_shared<Texture>(
        renderer,
        "_YAR_MISSING_EMISSIVE",
        4,
        pixel,
        TextureType::TEX_EMISSIVE,
        true
    );

    renderer->SetMissingTexture(TextureType::TEX_ALBEDO, missingAlbedo);
    renderer->SetMissingTexture(TextureType::TEX_ORM, missingMRAO);
    renderer->SetMissingTexture(TextureType::TEX_NORMAL, missingNormal);
    renderer->SetMissingTexture(TextureType::TEX_EMISSIVE, missingEmissive);
}

std::shared_ptr<INode> Assets::LoadGLTF(const char* path)
{
    auto renderer = static_pointer_cast<Renderer>(g_renderer);
    return std::make_shared<GLTFNode>(renderer, path);
}

std::shared_ptr<ISky> Assets::LoadSky(const char* folder)
{
    return std::make_shared<Sky>(folder);
}

std::shared_ptr<INode> Assets::CreateBox(const glm::vec3& extents, const glm::vec3& color)
{
    // clang-format off
    std::vector<VertexUnlit> vertices = {
        {.position = {-extents.x, -extents.y, -extents.z}, .color = color},
        {.position = { extents.x, -extents.y, -extents.z}, .color = color},
        {.position = {-extents.x, -extents.y,  extents.z}, .color = color},
        {.position = { extents.x, -extents.y,  extents.z}, .color = color},
        {.position = {-extents.x,  extents.y, -extents.z}, .color = color},
        {.position = { extents.x,  extents.y, -extents.z}, .color = color},
        {.position = {-extents.x,  extents.y,  extents.z}, .color = color},
        {.position = { extents.x,  extents.y,  extents.z}, .color = color},
    };
    std::vector<Index> indices = {
         5, 1, 0, 0, 4, 5,
         4, 0, 2, 2, 6, 4,
         6, 2, 3, 3, 7, 6,
         7, 3, 1, 1, 5, 7,
         3, 0, 1, 2, 0, 3,
         7, 5, 4, 4, 6, 7,
    };
    // clang-format on
    std::shared_ptr<IBuffer> vertexBuffer;
    std::shared_ptr<IBuffer> indexBuffer;
    g_renderer->CreateBuffer(
        vertexBuffer,
        VertexBuffer,
        vertices.data(),
        sizeof(VertexUnlit),
        static_cast<uint32_t>(vertices.size())
    );
    g_renderer->CreateBuffer(
        indexBuffer,
        IndexBuffer,
        indices.data(),
        sizeof(Index),
        static_cast<uint32_t>(indices.size())
    );
    auto mesh = std::make_shared<Mesh<VertexUnlit>>(vertices, indices, vertexBuffer, indexBuffer);
    // TODO this is silly
    auto renderer = static_pointer_cast<Renderer>(g_renderer);
    auto material = std::make_shared<Material>(
        "box",
        renderer->GetMissingTexture(TextureType::TEX_ALBEDO),
        renderer->GetMissingTexture(TextureType::TEX_ORM),
        renderer->GetMissingTexture(TextureType::TEX_NORMAL),
        renderer->GetMissingTexture(TextureType::TEX_EMISSIVE),
        1.0f,
        1.0f,
        nullptr
    );
    return std::make_shared<MeshNode<VertexUnlit>>("box", mesh, material);
}
}; // namespace yar
