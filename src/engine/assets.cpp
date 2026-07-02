#include "assets.h"

#include "../renderer/renderer.h"
#include "../renderer/texture.h"
#include "../world/gltf_node.h"
#include "../world/sky.h"

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
}; // namespace yar
