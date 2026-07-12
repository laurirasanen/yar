#pragma once

#include "../renderer/data_types.h"
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
        // clang-format off
        std::vector<ShaderVertex> vertices = {
            // FRONT 0 - 3
            {{ size.x,  size.y,  size.z},  {0.0f,  0.0f,  1.0f}, {}, {0.0f, 0.0f}},
            {{ size.x, -size.y,  size.z},  {0.0f,  0.0f,  1.0f}, {}, {0.0f, 1.0f}},
            {{-size.x,  size.y,  size.z},  {0.0f,  0.0f,  1.0f}, {}, {1.0f, 0.0f}},
            {{-size.x, -size.y,  size.z},  {0.0f,  0.0f,  1.0f}, {}, {1.0f, 1.0f}},

            // BACK 4 - 7
            {{ size.x,  size.y, -size.z},  {0.0f,  0.0f, -1.0f}, {}, {1.0f, 1.0f}},
            {{ size.x, -size.y, -size.z},  {0.0f,  0.0f, -1.0f}, {}, {1.0f, 0.0f}},
            {{-size.x,  size.y, -size.z},  {0.0f,  0.0f, -1.0f}, {}, {0.0f, 1.0f}},
            {{-size.x, -size.y, -size.z},  {0.0f,  0.0f, -1.0f}, {}, {0.0f, 0.0f}},

            // LEFT 8 - 11
            {{ size.x,   size.y, -size.z}, {1.0f,  0.0f,  0.0f}, {}, {1.0f, 1.0f}},
            {{ size.x,  -size.y, -size.z}, {1.0f,  0.0f,  0.0f}, {}, {1.0f, 0.0f}},
            {{ size.x,   size.y,  size.z}, {1.0f,  0.0f,  0.0f}, {}, {0.0f, 1.0f}},
            {{ size.x,  -size.y,  size.z}, {1.0f,  0.0f,  0.0f}, {}, {0.0f, 0.0f}},

            // RIGHT 12 - 15
            {{-size.x,  size.y, -size.z}, {-1.0f,  0.0f,  0.0f}, {}, {0.0f, 1.0f}},
            {{-size.x, -size.y, -size.z}, {-1.0f,  0.0f,  0.0f}, {}, {0.0f, 0.0f}},
            {{-size.x,  size.y,  size.z}, {-1.0f,  0.0f,  0.0f}, {}, {1.0f, 1.0f}},
            {{-size.x, -size.y,  size.z}, {-1.0f,  0.0f,  0.0f}, {}, {1.0f, 0.0f}},

            // TOP 16 - 19
            {{ size.x,  size.y,  size.z},  {0.0f,  1.0f,  0.0f}, {}, {1.0f, 1.0f}},
            {{ size.x,  size.y, -size.z},  {0.0f,  1.0f,  0.0f}, {}, {1.0f, 0.0f}},
            {{-size.x,  size.y,  size.z},  {0.0f,  1.0f,  0.0f}, {}, {0.0f, 1.0f}},
            {{-size.x,  size.y, -size.z},  {0.0f,  1.0f,  0.0f}, {}, {0.0f, 0.0f}},

            // BOTTOM 20 - 23
            {{ size.x, -size.y,  size.z},  {0.0f, -1.0f,  0.0f}, {}, {0.0f, 0.0f}},
            {{ size.x, -size.y, -size.z},  {0.0f, -1.0f,  0.0f}, {}, {0.0f, 1.0f}},
            {{-size.x, -size.y,  size.z},  {0.0f, -1.0f,  0.0f}, {}, {1.0f, 0.0f}},
            {{-size.x, -size.y, -size.z},  {0.0f, -1.0f,  0.0f}, {}, {1.0f, 1.0f}},
        };
        std::vector<uint32_t> indices = {
            // FRONT
            2,  3,  0,   0,  3,  1,
            // BACK
            4,  7,  6,   5,  7,  4,
            // LEFT
            10, 11, 8,   8,  11, 9,
            // RIGHT
            12, 15, 14,  13, 15, 12,
            // TOP
            16, 19, 18,  17, 19, 16,
            // BOTTOM
            22, 23, 20,  20, 23, 21,
        };
        // clang-format on
        auto indexBuffer = g_renderer->CreateBuffer(
            IndexBuffer,
            indices.data(),
            sizeof(uint32_t),
            static_cast<uint32_t>(indices.size())
        );
        Mesh::CalculateTangents(vertices, indices);

        auto shader = g_resources->Load<Shader>("uber.slang");

        auto material = Material(shader);

        auto aabb = AABB(vertices);

        m_mesh =
            std::make_shared<SubMesh>(std::move(vertices), std::move(indexBuffer), material, aabb);
    }

    const std::shared_ptr<SubMesh> GetSubMesh() const
    {
        return m_mesh;
    }

  private:
    std::shared_ptr<SubMesh> m_mesh;
};
}; // namespace yar
