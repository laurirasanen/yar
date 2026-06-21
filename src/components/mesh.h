#pragma once

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../log.h"
#include "../renderer/buffer.h"
#include "material.h"

namespace yar
{
enum VertexType
{
    Unlit,
    Shaded,
};

struct VertexUnlit
{
    glm::vec3 position;
    glm::vec3 color;
};

struct VertexShaded

{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

typedef uint32_t Index;

template<class T>
class Mesh
{
  public:
    Mesh() = delete;

    Mesh(
        std::vector<T>          vertices,
        std::vector<Index>      indices,
        std::shared_ptr<Buffer> vertexBuffer,
        std::shared_ptr<Buffer> indexBuffer
    ) :
        m_vertices(vertices),
        m_indices(indices),
        m_vertexBuffer(vertexBuffer),
        m_indexBuffer(indexBuffer)
    {
        for (const auto& vert : vertices)
        {
            m_min = glm::min(m_min, vert.position);
            m_max = glm::max(m_max, vert.position);
        }
        LOG_DEBUG(
            "Mesh min: [{:.2f}, {:.2f}, {:.2f}], max: [{:.2f}, {:.2f}, {:.2f}]",
            m_min.x,
            m_min.y,
            m_min.z,
            m_max.x,
            m_max.y,
            m_max.z
        );
    }

    ~Mesh()
    {
    }

    Mesh(const Mesh&)            = delete;
    Mesh(Mesh&&)                 = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&)      = delete;

    const std::vector<T>& GetVertices()
    {
        return m_vertices;
    }

    const std::vector<Index>& GetIndices()
    {
        return m_indices;
    }

    std::shared_ptr<Buffer> GetVertexBuffer()
    {
        return m_vertexBuffer;
    }

    std::shared_ptr<Buffer> GetIndexBuffer()
    {
        return m_indexBuffer;
    }

    glm::vec3 GetMin() const
    {
        return m_min;
    }

    glm::vec3 GetMax() const
    {
        return m_max;
    }

  private:
    VertexType m_vertexType;

    std::vector<T>     m_vertices;
    std::vector<Index> m_indices;

    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indexBuffer;

    std::shared_ptr<Material> m_material;

    glm::vec3 m_min;
    glm::vec3 m_max;
};
}; // namespace yar
