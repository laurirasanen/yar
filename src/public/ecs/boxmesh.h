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
    BoxMeshComponent(Entity* owner, const glm::vec3& size) : Component(owner)
    {
        const uint32_t vertexCount = 24;
        // clang-format off
        std::vector<float> vertices = {
            // FRONT 0 - 3
             size.x,  size.y,  size.z,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
             size.x, -size.y,  size.z,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -size.x,  size.y,  size.z,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            -size.x, -size.y,  size.z,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,

            // BACK 4 - 7
             size.x,  size.y, -size.z,  0.0f,  0.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
             size.x, -size.y, -size.z,  0.0f,  0.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            -size.x,  size.y, -size.z,  0.0f,  0.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -size.x, -size.y, -size.z,  0.0f,  0.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

            // LEFT 8 - 11
            size.x,   size.y, -size.z,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
            size.x,  -size.y, -size.z,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
            size.x,   size.y,  size.z,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
            size.x,  -size.y,  size.z,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

            // RIGHT 12 - 15
            -size.x,  size.y, -size.z, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
            -size.x, -size.y, -size.z, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
            -size.x,  size.y,  size.z, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
            -size.x, -size.y,  size.z, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,

            // TOP 16 - 19
             size.x,  size.y,  size.z,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
             size.x,  size.y, -size.z,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
            -size.x,  size.y,  size.z,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
            -size.x,  size.y, -size.z,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,

            // BOTTOM 20 - 23
             size.x, -size.y,  size.z,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
             size.x, -size.y, -size.z,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
            -size.x, -size.y,  size.z,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
            -size.x, -size.y, -size.z,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
        };
        std::vector<uint32_t> indices = {
            // FRONT
            0,  3,  2,  1,  3,  0,
            // BACK
            6,  7,  4,  4,  7,  5,
            // LEFT
            8,  11, 10, 9,  11, 8,
            // RIGHT
            14, 15, 12, 12, 15, 13,
            // TOP
            16, 19, 18, 17, 19, 16,
            // BOTTOM
            22, 23, 20, 20, 23, 21,
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
            vecs[i].x = vertices[i * 11 + 0];
            vecs[i].y = vertices[i * 11 + 1];
            vecs[i].z = vertices[i * 11 + 2];
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
