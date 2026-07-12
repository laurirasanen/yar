#pragma once

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../assets/gltf.h"
#include "../geometry.h"
#include "../material.h"
#include "../renderer/data_types.h"
#include "../renderer/ibuffer.h"
#include "../renderer/irenderer.h"
#include "resource.h"

namespace yar
{
class SubMesh
{
  public:
    SubMesh() = delete;

    SubMesh(
        std::shared_ptr<std::vector<ShaderVertex>> vertices,
        std::shared_ptr<IBuffer>                   indexBuffer,
        Material                                   material,
        AABB                                       aabb
    ) :
        m_vertexCount(static_cast<uint32_t>(vertices->size())),
        m_indexCount(indexBuffer->GetElementCount()),
        m_vertices(vertices),
        m_indexBuffer(indexBuffer),
        m_material(material),
        m_aabb(aabb)
    {
    }

    ~SubMesh()
    {
    }

    SubMesh(const SubMesh&)            = default;
    SubMesh(SubMesh&&)                 = default;
    SubMesh& operator=(const SubMesh&) = default;
    SubMesh& operator=(SubMesh&&)      = default;

    uint32_t GetVertexCount() const
    {
        return m_vertexCount;
    }

    uint32_t GetIndexCount() const
    {
        return m_indexCount;
    }

    const std::shared_ptr<std::vector<ShaderVertex>> GetVertices() const
    {
        return m_vertices;
    }

    const std::shared_ptr<IBuffer> GetIndexBuffer() const
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

    std::shared_ptr<std::vector<ShaderVertex>> m_vertices;
    std::shared_ptr<IBuffer>                   m_indexBuffer;

    Material m_material;

    AABB m_aabb;
};

class Mesh : public Resource
{
  public:
    Mesh(const std::string& path) : Resource(path)
    {
    }

    const std::vector<std::shared_ptr<SubMesh>>& GetSubMeshes() const
    {
        return m_subMeshes;
    }

  protected:
    bool DoLoad() override
    {
        std::vector<GltfData> data;

        if (!GLTF::Load(GetId(), data))
        {
            return false;
        }

        for (const auto& d : data)
        {
            const uint32_t vertexCount = static_cast<uint32_t>(d.positions.size() / 3);
            auto           vertices    = std::make_shared<std::vector<ShaderVertex>>();
            vertices->resize(vertexCount);
            for (uint32_t i = 0; i < vertexCount; i++)
            {
                (*vertices)[i].position.x = d.positions[i * 3 + 0];
                (*vertices)[i].position.y = d.positions[i * 3 + 1];
                (*vertices)[i].position.z = d.positions[i * 3 + 2];

                (*vertices)[i].normal.x = d.normals[i * 3 + 0];
                (*vertices)[i].normal.y = d.normals[i * 3 + 1];
                (*vertices)[i].normal.z = d.normals[i * 3 + 2];

                (*vertices)[i].tangent.x = d.tangents[i * 3 + 0];
                (*vertices)[i].tangent.y = d.tangents[i * 3 + 1];
                (*vertices)[i].tangent.z = d.tangents[i * 3 + 2];

                (*vertices)[i].uv.x = d.uvs[i * 2 + 0];
                (*vertices)[i].uv.y = d.uvs[i * 2 + 1];
            }

            auto indexBuffer = g_renderer->GetIndexBuffer(d.indices);

            auto shader = g_resources->Load<Shader>("uber.slang");

            std::vector<ResourceHandle<Texture>> textures =
                {d.textures.albedo, d.textures.orm, d.textures.normal, d.textures.emissive};
            std::vector<float> params = {
                1.0f,
                d.parameters.roughness,
                d.parameters.metallic,
                d.parameters.emissive[0],
                d.parameters.emissive[1],
                d.parameters.emissive[2]
            };
            auto material = Material(shader);
            material.SetTextures(textures);
            material.SetParameters(params);

            AABB aabb(vertices);

            auto sub = std::make_shared<SubMesh>(vertices, indexBuffer, material, aabb);
            m_subMeshes.push_back(sub);
        }

        return true;
    }

    bool DoUnload() override
    {
        m_subMeshes.clear();
        return true;
    }

  private:
    std::vector<std::shared_ptr<SubMesh>> m_subMeshes;
};
}; // namespace yar
