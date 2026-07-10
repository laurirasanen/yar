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
    BoxMeshComponent(const glm::vec3& size) : Component()
    {
        const uint32_t vertexCount = 8;
        // clang-format off
        std::vector<float> vertices = {
            -size.x, -size.y, -size.z,
             size.x, -size.y, -size.z,
            -size.x, -size.y,  size.z,
             size.x, -size.y,  size.z,
            -size.x,  size.y, -size.z,
             size.x,  size.y, -size.z,
            -size.x,  size.y,  size.z,
             size.x,  size.y,  size.z,
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
        auto indexBuffer = g_renderer->CreateBuffer(
            IndexBuffer,
            indices.data(),
            sizeof(uint32_t),
            static_cast<uint32_t>(indices.size())
        );

        auto vertShader = g_resources->Load<Shader>("uber.slang", SHADER_ENTRY_VERTEX);
        auto fragShader = g_resources->Load<Shader>("uber.slang", SHADER_ENTRY_PIXEL);

        auto material = Material(vertShader, fragShader);

        std::vector<glm::vec3> vecs = {};
        vecs.resize(vertexCount);
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            vecs[i].x = vertices[i * 3 + 0];
            vecs[i].y = vertices[i * 3 + 1];
            vecs[i].z = vertices[i * 3 + 2];
        }
        auto aabb = AABB(vecs);

        m_mesh = std::make_shared<SubMesh>(vertexCount, vertices, indexBuffer, material, aabb);
    }

    const std::shared_ptr<SubMesh> GetSubMesh() const
    {
        return m_mesh;
    }

  private:
    std::shared_ptr<SubMesh> m_mesh;
};
}; // namespace yar
