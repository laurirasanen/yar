#pragma once

#include "../public/inode.h"
#include "../public/irenderer.h"
#include "../renderer/material.h"
#include "../renderer/mesh.h"

namespace yar
{
template<class V>
class MeshNode : public IRenderNode
{
  public:
    MeshNode(
        std::string               name,
        std::shared_ptr<Mesh<V>>  mesh,
        std::shared_ptr<Material> material,
        RenderPipeline            pipeline
    ) :
        IRenderNode(name, material, pipeline),
        m_mesh(mesh)
    {
    }

    void UpdateAABB() override
    {
        const Transform t        = GetGlobalTransform();
        const auto      meshAABB = m_mesh->GetAABB().Transform(t);
        m_aabb.min               = meshAABB.min;
        m_aabb.max               = meshAABB.max;

        m_aabb.center = m_aabb.min + 0.5f * (m_aabb.max - m_aabb.min);

        for (const auto& child : m_children)
        {
            child->UpdateAABB();
        }
    }

    uint32_t GetVertexCount() const override
    {
        return m_mesh->GetVertexCount();
    }

    uint32_t GetIndexCount() const override
    {
        return m_mesh->GetIndexCount();
    }

    void Render() const override
    {
        g_renderer->DrawWithBuffers(m_mesh->GetVertexBuffer(), m_mesh->GetIndexBuffer());
    }

  private:
    std::shared_ptr<Mesh<V>> m_mesh;
};
}; // namespace yar
