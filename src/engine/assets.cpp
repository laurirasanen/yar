#include "assets.h"

#include "../public/renderer/irenderer.h"
#include "../renderer/renderer.h"
#include "../renderer/texture.h"
#include "../world/gltf_node.h"
#include "../world/mesh_node.h"
#include "../world/sky.h"

#include <memory>

namespace yar
{
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
    // TODO this is a bit silly
    auto renderer = static_pointer_cast<Renderer>(g_renderer);
    auto material = std::make_shared<
        MaterialComponent>("box", nullptr, nullptr, nullptr, nullptr, 1.0f, 1.0f, nullptr);
    return std::make_shared<MeshNode<VertexUnlit>>("box", mesh, material, RenderPipeline::UNLIT);
}
}; // namespace yar
