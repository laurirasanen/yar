#pragma once

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../log.h"
#include "../renderer/buffer.h"
#include "../renderer/data_types.h"
#include "material.h"

namespace yar
{
template<class T>
class Mesh
{
  public:
    Mesh() = delete;

    Mesh(
        std::vector<T>            vertices,
        std::vector<Index>        indices,
        std::shared_ptr<Buffer>   vertexBuffer,
        std::shared_ptr<Buffer>   indexBuffer,
        std::shared_ptr<Material> material
    ) :
        m_vertices(vertices),
        m_indices(indices),
        m_vertexBuffer(vertexBuffer),
        m_indexBuffer(indexBuffer),
        m_material(material)
    {
        for (const auto& vert : vertices)
        {
            m_min = glm::min(m_min, vert.position);
            m_max = glm::max(m_max, vert.position);
        }
    }

    ~Mesh()
    {
    }

    Mesh(const Mesh&)            = delete;
    Mesh(Mesh&&)                 = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&)      = delete;

    const std::vector<T>& GetVertices() const
    {
        return m_vertices;
    }

    const std::vector<Index>& GetIndices() const
    {
        return m_indices;
    }

    std::shared_ptr<Buffer> GetVertexBuffer() const
    {
        return m_vertexBuffer;
    }

    std::shared_ptr<Buffer> GetIndexBuffer() const
    {
        return m_indexBuffer;
    }

    std::shared_ptr<Material> GetMaterial() const
    {
        return m_material;
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
