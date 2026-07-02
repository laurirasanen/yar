#pragma once

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../public/ibuffer.h"
#include "../renderer/data_types.h"

namespace yar
{
template<class V>
class Mesh
{
  public:
    Mesh() = delete;

    Mesh(
        std::vector<V>           vertices,
        std::vector<Index>       indices,
        std::shared_ptr<IBuffer> vertexBuffer,
        std::shared_ptr<IBuffer> indexBuffer
    ) :
        m_vertices(vertices),
        m_indices(indices),
        m_vertexCount(vertexBuffer->GetElementCount()),
        m_indexCount(indexBuffer->GetElementCount()),
        m_vertexBuffer(vertexBuffer),
        m_indexBuffer(indexBuffer)
    {
        m_aabb = {
            .min    = vertices[0].position,
            .max    = vertices[0].position,
            .center = {},
        };
        for (size_t i = 1; i < vertices.size(); i++)
        {
            m_aabb.min = glm::min(m_aabb.min, vertices[i].position);
            m_aabb.max = glm::max(m_aabb.max, vertices[i].position);
        }
        m_aabb.center = m_aabb.min + 0.5f * (m_aabb.max - m_aabb.min);
    }

    ~Mesh()
    {
    }

    Mesh(const Mesh&)            = delete;
    Mesh(Mesh&&)                 = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&)      = delete;

    const std::vector<V>& GetVertices() const
    {
        return m_vertices;
    }

    const std::vector<Index>& GetIndices() const
    {
        return m_indices;
    }

    uint32_t GetVertexCount() const
    {
        return m_vertexCount;
    }

    uint32_t GetIndexCount() const
    {
        return m_indexCount;
    }

    std::shared_ptr<IBuffer> GetVertexBuffer() const
    {
        return m_vertexBuffer;
    }

    std::shared_ptr<IBuffer> GetIndexBuffer() const
    {
        return m_indexBuffer;
    }

    const AABB& GetAABB() const
    {
        return m_aabb;
    }

  private:
    std::vector<V>     m_vertices;
    std::vector<Index> m_indices;

    uint32_t m_vertexCount;
    uint32_t m_indexCount;

    std::shared_ptr<IBuffer> m_vertexBuffer;
    std::shared_ptr<IBuffer> m_indexBuffer;

    AABB m_aabb;
};
}; // namespace yar
