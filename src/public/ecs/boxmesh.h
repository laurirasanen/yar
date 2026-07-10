#pragma once

#include "../resource/mesh.h"
#include "../resource/shader.h"
#include "component.h"

#include <cstdint>

namespace yar
{
class BoxMeshComponent : public Component
{
  public:
    BoxMeshComponent(const glm::vec3& size, const glm::vec3& color) : Component()
    {
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
        };

        // clang-format off
        std::vector<Vertex> vertices = {
            {.position = {-size.x, -size.y, -size.z}, .color = color},
            {.position = { size.x, -size.y, -size.z}, .color = color},
            {.position = {-size.x, -size.y,  size.z}, .color = color},
            {.position = { size.x, -size.y,  size.z}, .color = color},
            {.position = {-size.x,  size.y, -size.z}, .color = color},
            {.position = { size.x,  size.y, -size.z}, .color = color},
            {.position = {-size.x,  size.y,  size.z}, .color = color},
            {.position = { size.x,  size.y,  size.z}, .color = color},
        };
        std::vector<uint32_t> indices = {
            5, 1, 0, 0, 4, 5,
            4, 0, 2, 2, 6, 4,
            6, 2, 3, 3, 7, 6,
            7, 3, 1, 1, 5, 7,
            3, 0, 1, 2, 0, 3,
            7, 5, 4, 4, 6, 7,
        };
        // clang-format on
        auto vertexBuffer = g_renderer->CreateBuffer(
            VertexBuffer,
            vertices.data(),
            sizeof(Vertex),
            static_cast<uint32_t>(vertices.size())
        );
        auto indexBuffer = g_renderer->CreateBuffer(
            IndexBuffer,
            indices.data(),
            sizeof(uint32_t),
            static_cast<uint32_t>(indices.size())
        );

        auto vertShader = g_resources->Load<Shader>("unlit.slang", SHADER_ENTRY_VERTEX);
        auto fragShader = g_resources->Load<Shader>("unlit.slang", SHADER_ENTRY_PIXEL);

        auto material = Material(vertShader, fragShader);

        std::vector<glm::vec3> positions = {};
        positions.reserve(vertices.size());
        for (const auto& v : vertices)
        {
            positions.push_back(v.position);
        }
        auto aabb = AABB(positions);

        m_mesh = SubMesh(vertexBuffer, indexBuffer, material, aabb);
    }

    const SubMesh& GetSubMesh() const
    {
        return m_mesh;
    }

  private:
    SubMesh m_mesh;
};
}; // namespace yar
