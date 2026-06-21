#pragma once

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../log.h"
#include "../renderer/buffer.h"
#include "../renderer/data_types.h"
#include "material.h"
#include "src/components/transform.h"

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
        m_aabb = {};
        for (const auto& vert : vertices)
        {
            m_aabb.min = glm::min(m_aabb.min, vert.position);
            m_aabb.max = glm::max(m_aabb.max, vert.position);
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

    void UpdateAABB(const Transform& transform)
    {
        m_globalAABB.min = transform.ToGlobalSpace(m_aabb.min);
        m_globalAABB.max = transform.ToGlobalSpace(m_aabb.min);

        /*
        LOG_DEBUG(
            "Mesh aabb min: [{:.2f}, {:.2f}, {:.2f}], max: [{:.2f}, {:.2f}, {:.2f}]",
            m_globalAABB.min.x,
            m_globalAABB.min.y,
            m_globalAABB.min.z,
            m_globalAABB.max.x,
            m_globalAABB.max.y,
            m_globalAABB.max.z
        );
        */
    }

    void FrustumCull(std::shared_ptr<Camera> camera)
    {
        Culled = !camera->IsInFrustum(m_globalAABB);
    }

    void MarkAsCulled(std::shared_ptr<Renderer> renderer)
    {
        renderer->AddCulledMesh(m_vertexBuffer, m_indexBuffer);
    }

    void Render(std::shared_ptr<Renderer> renderer, Transform& transform)
    {
        renderer->BindPipeline(RenderPipeline::SHADED);
        renderer->SetModelMatrix(transform);
        renderer->DrawWithBuffers(m_vertexBuffer, m_indexBuffer);
    }

    void RenderBounds(std::shared_ptr<Renderer> renderer)
    {
    }

    bool Culled;

  private:
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
