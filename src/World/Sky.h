#pragma once

#include <memory>

#include "../Components/Camera.h"
#include "../Renderer/Buffer.h"
#include "../Renderer/DataTypes.h"
#include "../Renderer/Renderer.h"
#include "Icosphere.h"

namespace drive
{
struct Sky
{
    std::shared_ptr<Buffer>   vertexBuffer;
    std::shared_ptr<Buffer>   indexBuffer;
    std::shared_ptr<Renderer> renderer;

    Sky(std::shared_ptr<Renderer> rend)
    {
        renderer       = rend;
        auto icosphere = Icosphere({}, CAM_FAR * 0.95f, 1, true);

        std::vector<Vertex_P> vertices;
        vertices.reserve(icosphere.positions.size());
        for (unsigned int i = 0; i < icosphere.positions.size(); i++)
        {
            vertices.push_back({icosphere.positions[i]});
        }

        renderer->CreateBuffer(
            vertexBuffer,
            VertexBuffer,
            vertices.data(),
            sizeof(Vertex_P),
            static_cast<uint32_t>(vertices.size())
        );
        renderer->CreateBuffer(
            indexBuffer,
            IndexBuffer,
            icosphere.indices.data(),
            sizeof(Index),
            static_cast<uint32_t>(icosphere.indices.size())
        );
    }

    void Render()
    {
        renderer->BindPipeline(RenderPipeline::SKY);
        renderer->DrawWithBuffers(vertexBuffer, indexBuffer);
    }
};
} // namespace drive
