#include <cstring>
#define CGLTF_IMPLEMENTATION
#include "../public/assets/gltf.h"

#include "../platform/memory.h"
#include "../public/log.h"
#include "../public/platform/fs.h"
#include "../public/util.h"

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <memory>

namespace yar
{
bool GLTF::Load(const std::string& path, std::vector<GltfData>& output)
{
    const auto  full_path = fs_relative_path(path);
    const char* cpath     = full_path.c_str();

    if (!fs_exists(full_path))
    {
        LOG_ERROR("No gltf found: {}", cpath);
        return false;
    }

    cgltf_options options {};
    cgltf_data*   data   = nullptr;
    cgltf_result  result = cgltf_parse_file(&options, cpath, &data);
    if (result != cgltf_result_success)
    {
        LOG_ERROR("Failed to parse gltf ({}): {}", static_cast<int>(result), cpath);
        cgltf_free(data);
        return false;
    }

    result = cgltf_validate(data);
    if (result != cgltf_result_success)
    {
        LOG_ERROR("Failed to validate gltf ({}): {}", static_cast<int>(result), cpath);
        cgltf_free(data);
        return false;
    }

    result = cgltf_load_buffers(&options, data, cpath);
    if (result != cgltf_result_success)
    {
        LOG_ERROR("Failed to read gltf buffers ({}): {}", static_cast<int>(result), cpath);
        cgltf_free(data);
        return false;
    }

    LOG_DEBUG("Loaded {} from scene {}", Memory::Pretty(data->file_size), cpath);

    size_t totalIndexCount  = 0;
    size_t totalVertexCount = 0;

    for (size_t i = 0; i < data->meshes_count; i++)
    {
        for (size_t primIdx = 0; primIdx < data->meshes[i].primitives_count; primIdx++)
        {
            const auto& primitive = data->meshes[i].primitives[primIdx];

            if (primitive.type != cgltf_primitive_type_triangles)
            {
                LOG_ERROR("Unhandled primitive type {}", static_cast<int>(primitive.type));
                continue;
            }

            GltfData parsedData;

            if (!ReadIndices(primitive, parsedData))
            {
                LOG_ERROR("Failed to read gltf indices: {}", cpath);
                cgltf_free(data);
                return false;
            }

            if (parsedData.indices.size() <= 0)
            {
                LOG_WARN("gltf has a primitive with no indices: {}", cpath);
                continue;
            }

            if (!ReadVertices(primitive, parsedData))
            {
                LOG_ERROR("Failed to read gltf vertices: {}", cpath);
                cgltf_free(data);
                return false;
            }

            CalculateTangents(parsedData);

            if (!ReadTextures(primitive, parsedData))
            {
                LOG_ERROR("Failed to read gltf textures: {}", cpath);
                cgltf_free(data);
                return false;
            }

            output.push_back(parsedData);

            totalIndexCount += parsedData.indices.size();
            totalVertexCount += parsedData.positions.size() / 3;
        }
    }

    LOG_DEBUG(
        "Parsed {}:\n"
        "  meshes: {}\n"
        "  verts: {}\n"
        "  indices: {}\n",
        cpath,
        output.size(),
        totalVertexCount,
        totalIndexCount
    );

    cgltf_free(data);

    return true;
}

bool GLTF::ReadIndices(const cgltf_primitive& primitive, GltfData& data)
{
    const auto indexCount =
        cgltf_accessor_unpack_indices(primitive.indices, nullptr, sizeof(uint32_t), 0);

    data.indices.resize(indexCount);

    auto readCount = cgltf_accessor_unpack_indices(
        primitive.indices,
        data.indices.data(),
        sizeof(uint32_t),
        indexCount
    );

    return readCount == indexCount;
}

bool GLTF::ReadVertices(const cgltf_primitive& primitive, GltfData& data)
{
    for (size_t attrIdx = 0; attrIdx < primitive.attributes_count; attrIdx++)
    {
        switch (primitive.attributes[attrIdx].type)
        {
            case cgltf_attribute_type_position:
            {
                if (!ReadFloats(primitive.attributes[attrIdx].data, data.positions))
                {
                    LOG_ERROR("Failed to read mesh positions");
                    return false;
                }
                break;
            }

            case cgltf_attribute_type_normal:
            {
                if (!ReadFloats(primitive.attributes[attrIdx].data, data.normals))
                {
                    LOG_ERROR("Failed to read mesh normals");
                    return false;
                }
                break;
            }

            case cgltf_attribute_type_texcoord:
            {
                if (!ReadFloats(primitive.attributes[attrIdx].data, data.uvs))
                {
                    LOG_ERROR("Failed to read mesh UVs");
                    return false;
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }

    if (data.positions.size() <= 0)
    {
        LOG_ERROR("mesh has no vertex positions");
        return false;
    }
    if (data.normals.size() <= 0)
    {
        LOG_ERROR("mesh has no vertex normals");
        return false;
    }
    if (data.uvs.size() <= 0)
    {
        LOG_ERROR("mesh has no vertex UVs");
        return false;
    }
    if (data.positions.size() % 3 != 0)
    {
        LOG_ERROR("mesh vertex positions count not multiple of 3 ({})", data.positions.size());
        return false;
    }
    if (data.normals.size() != data.positions.size())
    {
        LOG_ERROR(
            "mesh vertex normals invalid size ({}/{})",
            data.normals.size(),
            data.positions.size()
        );
        return false;
    }
    if (data.uvs.size() != 2 * data.positions.size() / 3)
    {
        LOG_ERROR("mesh vertex UVs invalid size ({}/{})", data.uvs.size(), data.positions.size());
        return false;
    }

    return true;
}

bool GLTF::ReadFloats(cgltf_accessor* accessor, std::vector<float>& floats)
{
    const auto floatCount = cgltf_accessor_unpack_floats(accessor, nullptr, 0);
    floats.resize(floatCount);
    const auto readCount = cgltf_accessor_unpack_floats(accessor, floats.data(), floatCount);
    return readCount == floatCount;
}

void GLTF::CalculateTangents(GltfData& data)
{
    const auto             vertexCount = data.positions.size() / 3;
    const auto             positions   = reinterpret_cast<glm::vec3*>(data.positions.data());
    auto                   normals     = reinterpret_cast<glm::vec3*>(data.normals.data());
    std::vector<glm::vec3> tangents    = {};
    std::vector<glm::vec3> bitangents  = {};
    const auto             uvs         = reinterpret_cast<glm::vec2*>(data.uvs.data());

    tangents.resize(vertexCount);
    bitangents.resize(vertexCount);

    for (size_t idx = 0; idx < data.indices.size(); idx += 3)
    {
        const auto idx0 = data.indices[idx];
        const auto idx1 = data.indices[idx + 1];
        const auto idx2 = data.indices[idx + 2];

        const auto edge1 = positions[idx1] - positions[idx0];
        const auto edge2 = positions[idx2] - positions[idx0];

        const auto uv1 = uvs[idx1] - uvs[idx0];
        const auto uv2 = uvs[idx2] - uvs[idx0];

        const auto r = 1.0f / (uv1.x * uv2.y - uv2.x * uv1.y);

        const auto tangent   = r * (edge1 * uv2.y - edge2 * uv1.y);
        const auto bitangent = -r * (edge2 * uv1.x - edge1 * uv2.x);

        tangents[idx0] += tangent;
        tangents[idx1] += tangent;
        tangents[idx2] += tangent;
        bitangents[idx0] += bitangent;
        bitangents[idx1] += bitangent;
        bitangents[idx2] += bitangent;
    }

    for (size_t v = 0; v < vertexCount; v++)
    {
        const auto cross = glm::cross(normals[v], tangents[v]);
        const auto sign  = glm::dot(cross, bitangents[v]) < 0 ? -1.0f : 1.0f;
        tangents[v] =
            glm::normalize(tangents[v] - normals[v] * glm::dot(normals[v], tangents[v])) * sign;
    }

    data.tangents.resize(vertexCount * 3);
    memcpy(data.tangents.data(), tangents.data(), data.tangents.size() * sizeof(float));
}

bool GLTF::ReadTextures(const cgltf_primitive& primitive, GltfData& data)
{
    if (!primitive.material)
    {
        LOG_ERROR("Primitive has no material");
        return false;
    }

    cgltf_texture_view* albedoView   = nullptr;
    cgltf_texture_view* ormView      = nullptr;
    cgltf_texture_view* normalView   = &primitive.material->normal_texture;
    cgltf_texture_view* emissiveView = &primitive.material->emissive_texture;

    data.parameters.metallic  = 0.0f;
    data.parameters.roughness = 0.8f;

    if (primitive.material->has_pbr_metallic_roughness)
    {
        albedoView = &primitive.material->pbr_metallic_roughness.base_color_texture;
        ormView    = &primitive.material->pbr_metallic_roughness.metallic_roughness_texture;

        data.parameters.metallic  = primitive.material->pbr_metallic_roughness.metallic_factor;
        data.parameters.roughness = primitive.material->pbr_metallic_roughness.roughness_factor;
    }

    for (uint32_t i = 0; i < 3; i++)
    {
        data.parameters.emissive[i] = primitive.material->emissive_factor[i];
        if (primitive.material->has_emissive_strength)
        {
            data.parameters.emissive[i] *= primitive.material->emissive_strength.emissive_strength;
        }
    }

    data.textures.albedo   = ReadTexture(albedoView, TextureType::TEX_ALBEDO);
    data.textures.orm      = ReadTexture(ormView, TextureType::TEX_ORM);
    data.textures.normal   = ReadTexture(normalView, TextureType::TEX_NORMAL);
    data.textures.emissive = ReadTexture(emissiveView, TextureType::TEX_EMISSIVE);
    return true;
}

ResourceHandle<Texture> GLTF::ReadTexture(const cgltf_texture_view* view, TextureType type)
{
    if (!view)
    {
        LOG_DEBUG("Null texture view");
        return ResourceHandle<Texture>();
    }

    if (!view->texture)
    {
        LOG_DEBUG("Texture view has no texture");
        return ResourceHandle<Texture>();
    }

    if (!view->texture->image)
    {
        LOG_ERROR("Texture has no image");
        return ResourceHandle<Texture>();
    }

    if (!view->texture->image->buffer_view)
    {
        LOG_ERROR("Image has no buffer view");
        return ResourceHandle<Texture>();
    }

    std::string name;
    if (view->texture->name)
    {
        name = std::string {view->texture->name};
    }
    else if (view->texture->image->name)
    {
        name = std::string {view->texture->image->name};
    }
    else
    {
        RAND_STR(16, name);
        LOG_WARN("Texture has no name, rand: {}", name);
    }

    LOG_DEBUG("Loading texture {}", name);

    const auto data =
        static_cast<const void*>(cgltf_buffer_view_data(view->texture->image->buffer_view));
    const auto size = static_cast<size_t>(view->texture->image->buffer_view->size);

    return g_resources->Load<Texture>(name, type, size, data);
}
}; // namespace yar
