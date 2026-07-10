#pragma once

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../assets/gltf.h"
#include "../geometry.h"
#include "../material.h"
#include "../renderer/ibuffer.h"
#include "../renderer/irenderer.h"
#include "resource.h"
#include "src/public/shader/compiler.h"

namespace yar
{
class SubMesh
{
  public:
    SubMesh() = delete;

    SubMesh(
        std::shared_ptr<IBuffer> vertexBuffer,
        std::shared_ptr<IBuffer> indexBuffer,
        Material                 material,
        AABB                     aabb
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

    const Material& GetMaterial() const
    {
        return m_material;
    }

  private:
    uint32_t m_vertexCount;
    uint32_t m_indexCount;

    std::shared_ptr<IBuffer> m_vertexBuffer;
    std::shared_ptr<IBuffer> m_indexBuffer;

    Material m_material;

    AABB m_aabb;
};

class Mesh : public Resource
{
  public:
    Mesh(const std::string& path) : Resource(path)
    {
    }

    const std::vector<SubMesh>& GetSubMeshes() const
    {
        return m_subMeshes;
    }

  protected:
    bool DoLoad() override
    {
        std::vector<GltfData> data;

        if (!GLTF::Load(m_path, data))
        {
            return false;
        }

        for (const auto& d : data)
        {
            auto vertexBuffer =
                g_renderer->GetVertexBuffer(d.positions, d.normals, d.tangents, d.uvs);
            auto indexBuffer = g_renderer->GetIndexBuffer(d.indices);

            auto vertShader = g_resources->Load<Shader>("pbr", SHADER_ENTRY_VERTEX);
            auto fragShader = g_resources->Load<Shader>("pbr", SHADER_ENTRY_PIXEL);

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

    bool DoUnload() override
    {
        m_subMeshes.clear();
        return true;
    }

  private:
    std::vector<SubMesh> m_subMeshes;
};
}; // namespace yar
