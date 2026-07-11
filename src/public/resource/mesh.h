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
        uint32_t                 vertexCount,
        std::vector<float>       vertices,
        std::shared_ptr<IBuffer> indexBuffer,
        Material                 material,
        AABB                     aabb
    ) :
        m_vertexCount(vertexCount),
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

    const std::vector<float>& GetVertices() const
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

    std::vector<float>       m_vertices;
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

        if (!GLTF::Load(GetId(), data))
        {
            return false;
        }

        for (const auto& d : data)
        {
            const uint32_t     vertexCount = static_cast<uint32_t>(d.positions.size() / 3);
            std::vector<float> vertices;
            for (uint32_t i = 0; i < vertexCount; i++)
            {
                vertices.push_back(d.positions[i * 3 + 0]);
                vertices.push_back(d.positions[i * 3 + 1]);
                vertices.push_back(d.positions[i * 3 + 2]);

                vertices.push_back(d.normals[i * 3 + 0]);
                vertices.push_back(d.normals[i * 3 + 1]);
                vertices.push_back(d.normals[i * 3 + 2]);

                vertices.push_back(d.tangents[i * 3 + 0]);
                vertices.push_back(d.tangents[i * 3 + 1]);
                vertices.push_back(d.tangents[i * 3 + 2]);

                vertices.push_back(d.uvs[i * 2 + 0]);
                vertices.push_back(d.uvs[i * 2 + 1]);
            }

            auto indexBuffer = g_renderer->GetIndexBuffer(d.indices);

            auto vertShader = g_resources->Load<Shader>("pbr", SHADER_ENTRY_VERTEX);
            auto fragShader = g_resources->Load<Shader>("pbr", SHADER_ENTRY_PIXEL);

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
            auto material = Material(vertShader, fragShader);
            material.SetTextures(textures);
            material.SetParameters(params);

            std::vector<glm::vec3> vecs = {};
            vecs.resize(vertexCount);
            for (uint32_t i = 0; i < vertexCount; i++)
            {
                vecs[i].x = d.positions[i * 3 + 0];
                vecs[i].y = d.positions[i * 3 + 1];
                vecs[i].z = d.positions[i * 3 + 2];
            }
            AABB aabb(vecs);

            m_subMeshes.emplace_back(vertexCount, vertices, indexBuffer, material, aabb);
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
