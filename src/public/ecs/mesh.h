#pragma once

#include "../public/resource/mesh.h"
#include "component.h"

namespace yar
{
class MeshComponent : public Component
{
  public:
    MeshComponent(ResourceHandle<Mesh> mesh) : Component(), m_mesh(mesh),
    {
    }

    void Render() override
    {
        const auto& meshes = m_mesh->GetSubMeshes();
        for (const auto& mesh : meshes)
        {
            if (mesh->IsCulled())
            {
                continue;
            }
            g_renderer->DrawWithBuffers(mesh->GetVertexBuffer(), mesh->GetIndexBuffer());
        }
    }

    uint32_t GetIndexCount() const
    {
        uint32_t    count  = 0;
        const auto& meshes = m_mesh->GetSubMeshes();
        for (const auto& mesh : meshes)
        {
            count += mesh.GetIndexCount();
        }
        return count;
    }

    uint32_t GetVertexCount() const
    {
        uint32_t    count  = 0;
        const auto& meshes = m_mesh->GetSubMeshes();
        for (const auto& mesh : meshes)
        {
            count += mesh.GetVertexCount();
        }
        return count;
    }

    const ResourceHandle<Mesh>& GetMesh() const
    {
        return m_mesh;
    }

  private:
    ResourceHandle<Mesh> m_mesh;
};
}; // namespace yar
