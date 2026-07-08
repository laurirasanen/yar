#pragma once

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../geometry.h"
#include "../public/ibuffer.h"
#include "../renderer/data_types.h"
#include "resource.h"

namespace yar
{
class SubMesh
{
  public:
    SubMesh() = delete;

    SubMesh(
        std::shared_ptr<IBuffer>  vertexBuffer,
        std::shared_ptr<IBuffer>  indexBuffer,
        std::unique_ptr<Material> material,
        AABB                      aabb
    ) :
        m_vertexCount(vertexBuffer->GetElementCount()),
        m_indexCount(indexBuffer->GetElementCount()),
        m_vertexBuffer(vertexBuffer),
        m_indexBuffer(indexBuffer),
        m_material(material),
        m_aabb(aabb)
    {
    }

    ~SubMesh()
    {
    }

    SubMesh(const SubMesh&)            = delete;
    SubMesh(SubMesh&&)                 = delete;
    SubMesh& operator=(const SubMesh&) = delete;
    SubMesh& operator=(SubMesh&&)      = delete;

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
    uint32_t m_vertexCount;
    uint32_t m_indexCount;

    std::shared_ptr<IBuffer> m_vertexBuffer;
    std::shared_ptr<IBuffer> m_indexBuffer;

    std::unique_ptr<Material> m_material;

    AABB m_aabb;
};

class Mesh : public Resource
{
  public:
    Mesh(const std::string& path) : Resource(path)
    {
    }

  protected:
    bool DoLoad()
    {
        std::vector<GltfData> data;

        if (!gltf::Load(m_path, data))
        {
            return false;
        }

        for (const auto& d : data)
        {
            auto vertexBuffer =
                g_renderer->GetVertexBuffer(d.positions, d.normals, d.tangents, d.uvs);
            auto indexBuffer = g_renderer->GetIndexBuffer(d.indices);

            auto vertShader = g_resources->Load<Shader>("uber", ShaderType::Vertex);
            auto fragShader = g_resources->Load<Shader>("uber", ShaderType::Fragment);

            auto material = Material(vertShader, fragShader);
            material->SetAlbedo(d.textures.albedo);
            material->SetORM(d.textures.orm);
            material->SetNormal(d.textures.normal);
            material->SetEmissive(d.textures.normal);
            material->SetMetallic(d.parameters.metallic);
            material->SetRoughness(d.parameters.roughness);
            material->SetEmissiveFactor(d.parameters.emissive);

            auto aabb = AABB(d.positions);

            m_subMeshes.push_back(vertexBuffer, indexBuffer, material, aabb);
        }

        return true;
    }

    bool DoUnload()
    {
        m_subMeshes.clear();
        return true;
    }

  private:
    std::vector<SubMesh> m_subMeshes;
};
}; // namespace yar
