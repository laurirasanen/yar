#pragma once

#include "../public/inode.h"
#include "../public/irenderer.h"
#include "../renderer/material.h"
#include "../renderer/mesh.h"

namespace yar
{
template<class V>
class MeshNode : public INode
{
  public:
    MeshNode(std::string name, std::shared_ptr<Mesh<V>> mesh, std::shared_ptr<Material> material) :
        INode(name),
        m_mesh(mesh),
        m_material(material)
    {
    }

    void UpdateAABB() override
    {
        m_aabb.min = {};
        m_aabb.max = {};

        for (const auto& child : m_children)
        {
            child->UpdateAABB();
            const auto aabb = child->GetAABB();
            m_aabb.min      = glm::min(m_aabb.min, aabb.min);
            m_aabb.max      = glm::max(m_aabb.max, aabb.max);
        }

        const Transform t        = GetGlobalTransform();
        const auto      meshAABB = m_mesh->GetAABB().Transform(t);
        m_aabb.min               = glm::min(m_aabb.min, meshAABB.min);
        m_aabb.max               = glm::max(m_aabb.max, meshAABB.max);

        m_aabb.center = m_aabb.min + 0.5f * (m_aabb.max - m_aabb.min);
    }

    bool Renderable() const override
    {
        return true;
    }

    uint32_t GetVertexCount() const override
    {
        return m_mesh->GetVertexCount();
    }

    uint32_t GetIndexCount() const override
    {
        return m_mesh->GetIndexCount();
    }

    std::shared_ptr<IMaterial> GetMaterial() override
    {
        return m_material;
    }

    void Render() override
    {
        g_renderer->DrawWithBuffers(m_mesh->GetVertexBuffer(), m_mesh->GetIndexBuffer());
    }

  private:
    std::shared_ptr<Mesh<V>>  m_mesh;
    std::shared_ptr<Material> m_material;
};
}; // namespace yar
