#pragma once

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../renderer/data_types.h"
#include "../renderer/vulkan/buffer.h"
#include "material.h"
#include "transform.h"

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
        m_transform = std::make_shared<Transform>();
        m_aabb      = {
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
        m_globalAABB  = m_aabb;
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

    std::shared_ptr<Transform> GetTransform() const
    {
        return m_transform;
    }

    std::shared_ptr<Material> GetMaterial() const
    {
        return m_material;
    }

    void UpdateAABB()
    {
        const glm::vec3 corners[8] = {
            m_transform->ToGlobalSpace(glm::vec4(m_aabb.min.x, m_aabb.min.y, m_aabb.min.z, 1.0f)),
            m_transform->ToGlobalSpace(glm::vec4(m_aabb.max.x, m_aabb.min.y, m_aabb.min.z, 1.0f)),
            m_transform->ToGlobalSpace(glm::vec4(m_aabb.min.x, m_aabb.max.y, m_aabb.min.z, 1.0f)),
            m_transform->ToGlobalSpace(glm::vec4(m_aabb.max.x, m_aabb.max.y, m_aabb.min.z, 1.0f)),
            m_transform->ToGlobalSpace(glm::vec4(m_aabb.min.x, m_aabb.min.y, m_aabb.max.z, 1.0f)),
            m_transform->ToGlobalSpace(glm::vec4(m_aabb.max.x, m_aabb.min.y, m_aabb.max.z, 1.0f)),
            m_transform->ToGlobalSpace(glm::vec4(m_aabb.min.x, m_aabb.max.y, m_aabb.max.z, 1.0f)),
            m_transform->ToGlobalSpace(glm::vec4(m_aabb.max.x, m_aabb.max.y, m_aabb.max.z, 1.0f)),
        };

        m_globalAABB.min = corners[0];
        m_globalAABB.max = corners[0];

        for (uint8_t i = 1; i < 8; i++)
        {
            m_globalAABB.min = glm::min(m_globalAABB.min, corners[i]);
            m_globalAABB.max = glm::max(m_globalAABB.max, corners[i]);
        }

        m_globalAABB.center = m_globalAABB.min + 0.5f * (m_globalAABB.max - m_globalAABB.min);
    }

    const AABB& GetAABB() const
    {
        return m_globalAABB;
    }

    void FrustumCull(std::shared_ptr<Camera> camera)
    {
        Culled = !camera->IsInFrustum(m_globalAABB);
    }

    bool Culled;

  private:
    std::shared_ptr<Transform> m_transform;

    VertexType m_vertexType;

    std::vector<T>     m_vertices;
    std::vector<Index> m_indices;

    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indexBuffer;

    std::shared_ptr<Material> m_material;

    AABB m_aabb;
    AABB m_globalAABB;
};
}; // namespace yar
