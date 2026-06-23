#define CGLTF_IMPLEMENTATION
#include "scene.h"

#include "../log.h"
#include "../platform/fs.h"
#include "../platform/memory.h"
#include "../util.h"

#include <glm/geometric.hpp>

namespace yar
{
Scene::Scene(std::shared_ptr<Renderer> renderer, std::shared_ptr<UI> ui, std::string path) :
    m_path(path)
{
    const auto  full_path = fs_relative_path(path);
    const char* cpath     = full_path.c_str();
    LOG_DEBUG("Loading scene {}", cpath);
    ui->SetLoadingScene(cpath);

    if (!fs_exists(full_path))
    {
        LOG_ERROR("No scene found: {}", cpath);
        return;
    }

    cgltf_options options {};
    cgltf_data*   data   = nullptr;
    cgltf_result  result = cgltf_parse_file(&options, cpath, &data);
    if (result != cgltf_result_success)
    {
        LOG_ERROR("Failed to parse gltf ({}): {}", static_cast<int>(result), cpath);
        cgltf_free(data);
        return;
    }

    result = cgltf_validate(data);
    if (result != cgltf_result_success)
    {
        LOG_ERROR("Failed to validate gltf ({}): {}", static_cast<int>(result), cpath);
        cgltf_free(data);
        return;
    }

    result = cgltf_load_buffers(&options, data, cpath);
    if (result != cgltf_result_success)
    {
        LOG_ERROR("Failed to read gltf buffers ({}): {}", static_cast<int>(result), cpath);
        cgltf_free(data);
        return;
    }

    LOG_DEBUG("Loaded {} from scene {}", Memory::Pretty(data->file_size), cpath);

    size_t totalIndexCount  = 0;
    size_t totalVertexCount = 0;
    size_t totalPrimCount   = 0;
    size_t loadedPrimCount  = 0;

    for (size_t i = 0; i < data->meshes_count; i++)
    {
        totalPrimCount += data->meshes[i].primitives_count;
    }

    for (size_t i = 0; i < data->meshes_count; i++)
    {
        for (size_t primIdx = 0; primIdx < data->meshes[i].primitives_count; primIdx++)
        {
            loadedPrimCount++;
            ui->SetLoadingMesh(std::format("{}/{}", loadedPrimCount, totalPrimCount));

            const auto&               primitive = data->meshes[i].primitives[primIdx];
            std::vector<Index>        indices   = {};
            std::vector<VertexShaded> vertices  = {};

            if (primitive.type != cgltf_primitive_type_triangles)
            {
                LOG_ERROR("Unhandled primitive type {}", static_cast<int>(primitive.type));
                continue;
            }

            if (!ReadIndices(primitive, indices))
            {
                LOG_ERROR("Failed to read gltf indices: {}", cpath);
                cgltf_free(data);
                return;
            }

            if (indices.size() <= 0)
            {
                LOG_WARN("gltf has a primitive with no indices");
                continue;
            }

            if (!ReadVertices(primitive, vertices))
            {
                LOG_ERROR("Failed to read gltf vertices: {}", cpath);
                cgltf_free(data);
                return;
            }

            if (vertices.size() <= 0)
            {
                LOG_WARN("gltf has a primitive with no vertices");
                continue;
            }

            std::vector<glm::vec3> bitangents = {};
            bitangents.resize(vertices.size());

            for (size_t idx = 0; idx < indices.size(); idx += 3)
            {
                const auto idx0 = indices[idx];
                const auto idx1 = indices[idx + 1];
                const auto idx2 = indices[idx + 2];

                const auto& v0 = vertices[idx0];
                const auto& v1 = vertices[idx1];
                const auto& v2 = vertices[idx2];

                const auto edge1 = v1.position - v0.position;
                const auto edge2 = v2.position - v0.position;

                const auto uv1 = v1.uv - v0.uv;
                const auto uv2 = v2.uv - v0.uv;

                const auto r = 1.0f / (uv1.x * uv2.y - uv2.x * uv1.y);

                const auto tangent   = r * (edge1 * uv2.y - edge2 * uv1.y);
                const auto bitangent = -r * (edge2 * uv1.x - edge1 * uv2.x);

                vertices[idx0].tangent += tangent;
                vertices[idx1].tangent += tangent;
                vertices[idx2].tangent += tangent;
                bitangents[idx0] += bitangent;
                bitangents[idx1] += bitangent;
                bitangents[idx2] += bitangent;
            }

            for (size_t v = 0; v < vertices.size(); v++)
            {
                auto&       vert      = vertices[v];
                const auto& bitangent = bitangents[v];
                const auto  cross     = glm::cross(vert.normal, vert.tangent);
                const auto  sign      = glm::dot(cross, bitangent) < 0 ? -1.0f : 1.0f;
                vert.tangent =
                    glm::normalize(vert.tangent - vert.normal * glm::dot(vert.normal, vert.tangent))
                    * sign;
            }

            std::shared_ptr<Buffer> vertexBuffer;
            renderer->CreateBuffer(
                vertexBuffer,
                VertexBuffer,
                vertices.data(),
                sizeof(VertexShaded),
                static_cast<uint32_t>(vertices.size())
            );
            std::shared_ptr<Buffer> indexBuffer;
            renderer->CreateBuffer(
                indexBuffer,
                IndexBuffer,
                indices.data(),
                sizeof(Index),
                static_cast<uint32_t>(indices.size())
            );

            std::shared_ptr<Material> material = ReadMaterial(renderer, ui, primitive);

            auto mesh = std::make_shared<Mesh<VertexShaded>>(
                vertices,
                indices,
                vertexBuffer,
                indexBuffer,
                material
            );
            m_meshes.push_back(mesh);

            totalIndexCount += indices.size();
            totalVertexCount += vertices.size();
        }
    }

    LOG_DEBUG(
        "Parsed {}:\n"
        "  meshes: {}\n"
        "  verts: {}\n"
        "  indices: {}\n"
        "  materials: {}\n"
        "  textures: {}",
        cpath,
        m_meshes.size(),
        totalVertexCount,
        totalIndexCount,
        m_materials.size(),
        m_textures.size()
    );

    cgltf_free(data);
}

Scene::~Scene()
{
}

bool Scene::ReadIndices(const cgltf_primitive& primitive, std::vector<Index>& indices)
{
    const auto indexCount =
        cgltf_accessor_unpack_indices(primitive.indices, nullptr, sizeof(Index), 0);

    indices.resize(indexCount);

    auto readCount =
        cgltf_accessor_unpack_indices(primitive.indices, indices.data(), sizeof(Index), indexCount);

    return readCount == indexCount;
}

bool Scene::ReadVertices(const cgltf_primitive& primitive, std::vector<VertexShaded>& vertices)
{
    std::vector<float> vertPositions = {};
    std::vector<float> vertNormals   = {};
    std::vector<float> vertUVs       = {};

    for (size_t attrIdx = 0; attrIdx < primitive.attributes_count; attrIdx++)
    {
        switch (primitive.attributes[attrIdx].type)
        {
            case cgltf_attribute_type_position:
            {
                if (!ReadFloats(primitive.attributes[attrIdx].data, vertPositions))
                {
                    LOG_ERROR("Failed to read mesh positions");
                    return false;
                }
                break;
            }

            case cgltf_attribute_type_normal:
            {
                if (!ReadFloats(primitive.attributes[attrIdx].data, vertNormals))
                {
                    LOG_ERROR("Failed to read mesh normals");
                    return false;
                }
                break;
            }

            case cgltf_attribute_type_texcoord:
            {
                if (!ReadFloats(primitive.attributes[attrIdx].data, vertUVs))
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

    if (vertPositions.size() <= 0)
    {
        LOG_ERROR("mesh has no vertex positions");
        return false;
    }
    if (vertNormals.size() <= 0)
    {
        LOG_ERROR("mesh has no vertex normals");
        return false;
    }
    if (vertUVs.size() <= 0)
    {
        LOG_ERROR("mesh has no vertex UVs");
        return false;
    }
    if (vertPositions.size() % 3 != 0)
    {
        LOG_ERROR("mesh vertex positions count not multiple of 3 ({})", vertPositions.size());
        return false;
    }
    if (vertNormals.size() != vertPositions.size())
    {
        LOG_ERROR(
            "mesh vertex normals invalid size ({}/{})",
            vertNormals.size(),
            vertPositions.size()
        );
        return false;
    }
    if (vertUVs.size() != 2 * vertPositions.size() / 3)
    {
        LOG_ERROR("mesh vertex UVs invalid size ({}/{})", vertUVs.size(), vertPositions.size());
        return false;
    }

    const size_t vertexCount = vertPositions.size() / 3;

    vertices.resize(vertexCount);

    for (size_t vertIdx = 0; vertIdx < vertexCount; vertIdx++)
    {
        vertices[vertIdx].position.x = vertPositions[vertIdx * 3 + 0];
        vertices[vertIdx].position.y = vertPositions[vertIdx * 3 + 1];
        vertices[vertIdx].position.z = vertPositions[vertIdx * 3 + 2];

        vertices[vertIdx].normal.x = vertNormals[vertIdx * 3 + 0];
        vertices[vertIdx].normal.y = vertNormals[vertIdx * 3 + 1];
        vertices[vertIdx].normal.z = vertNormals[vertIdx * 3 + 2];

        vertices[vertIdx].uv.x = vertUVs[vertIdx * 2 + 0];
        vertices[vertIdx].uv.y = vertUVs[vertIdx * 2 + 1];
    }

    return true;
}

bool Scene::ReadFloats(cgltf_accessor* accessor, std::vector<float>& floats)
{
    const auto floatCount = cgltf_accessor_unpack_floats(accessor, nullptr, 0);
    floats.resize(floatCount);
    const auto readCount = cgltf_accessor_unpack_floats(accessor, floats.data(), floatCount);
    return readCount == floatCount;
}

std::shared_ptr<Material> Scene::ReadMaterial(
    std::shared_ptr<Renderer> renderer,
    std::shared_ptr<UI>       ui,
    const cgltf_primitive&    primitive
)
{
    if (!primitive.material)
    {
        LOG_ERROR("Primitive has no material");
        return nullptr;
    }

    std::string name;
    if (primitive.material->name)
    {
        name = std::string {primitive.material->name};
    }
    else
    {
        RAND_STR(16, name);
        LOG_WARN("Material has no name, rand: {}", name);
    };

    ui->SetLoadingMaterial(name);

    for (const auto& mat : m_materials)
    {
        if (mat->Name() == name)
        {
            return mat;
        }
    }

    LOG_DEBUG("Loading material {}", name);

    cgltf_texture_view* albedoView = nullptr;
    cgltf_texture_view* normalView = &primitive.material->normal_texture;
    cgltf_texture_view* ormView    = nullptr;

    float metalnessFactor = 1.0f;
    float roughnessFactor = 1.0f;

    if (primitive.material->has_pbr_metallic_roughness)
    {
        albedoView      = &primitive.material->pbr_metallic_roughness.base_color_texture;
        ormView         = &primitive.material->pbr_metallic_roughness.metallic_roughness_texture;
        metalnessFactor = primitive.material->pbr_metallic_roughness.metallic_factor;
        roughnessFactor = primitive.material->pbr_metallic_roughness.roughness_factor;
    }
    else if (primitive.material->has_pbr_specular_glossiness)
    {
        albedoView = &primitive.material->pbr_specular_glossiness.diffuse_texture;
    }

    auto albedoTex = ReadTexture(renderer, ui, albedoView);
    auto normalTex = ReadTexture(renderer, ui, normalView);
    auto ormTex    = ReadTexture(renderer, ui, ormView);
    if (!albedoTex)
    {
        albedoTex = renderer->GetMissingTexture(TextureType::TEX_ALBEDO);
    }
    if (!normalTex)
    {
        normalTex = renderer->GetMissingTexture(TextureType::TEX_NORMAL);
    }
    if (!ormTex)
    {
        ormTex = renderer->GetMissingTexture(TextureType::TEX_ORM);
    }

    m_materials.push_back(
        std::make_shared<
            Material>(name, albedoTex, normalTex, ormTex, metalnessFactor, roughnessFactor)
    );
    return m_materials.back();
}

std::shared_ptr<Texture> Scene::ReadTexture(
    std::shared_ptr<Renderer> renderer,
    std::shared_ptr<UI>       ui,
    const cgltf_texture_view* view
)
{
    if (!view)
    {
        LOG_DEBUG("Null texture view");
        return nullptr;
    }

    if (!view->texture)
    {
        LOG_DEBUG("Texture view has no texture");
        return nullptr;
    }

    if (!view->texture->image)
    {
        LOG_ERROR("Texture has no image");
        return nullptr;
    }

    if (!view->texture->image->buffer_view)
    {
        LOG_ERROR("Image has no buffer view");
        return nullptr;
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

    ui->SetLoadingTexture(name);

    for (const auto& tex : m_textures)
    {
        if (tex->Name() == name)
        {
            return tex;
        }
    }

    LOG_DEBUG("Loading texture {}", name);

    const auto mime = view->texture->image->mime_type;
    const auto data =
        static_cast<const void*>(cgltf_buffer_view_data(view->texture->image->buffer_view));
    const auto size   = static_cast<size_t>(view->texture->image->buffer_view->size);
    glm::vec2  offset = {0.0f, 0.0f};
    glm::vec2  scale  = {1.0f, 1.0f};

    if (view->has_transform)
    {
        offset.x = view->transform.offset[0];
        offset.y = view->transform.offset[1];
        scale.x  = view->transform.scale[0];
        scale.y  = view->transform.scale[1];
    }

    TextureFormat type = TextureFormat::FMT_UNKNOWN;
    if (std::strcmp(mime, "image/jpeg") == 0)
    {
        type = TextureFormat::FMT_SRGB;
    }
    else if (std::strcmp(mime, "image/png") == 0)
    {
        type = TextureFormat::FMT_SRGB;
    }
    else
    {
        throw std::runtime_error(std::format("Unkown mime type {}", mime));
    }

    m_textures.push_back(std::make_shared<Texture>(renderer, name, size, data, type));
    return m_textures.back();
}
}; // namespace yar
