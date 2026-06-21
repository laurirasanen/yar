#define CGLTF_IMPLEMENTATION

#include "model.h"
#include "../log.h"
#include "../platform/fs.h"
#include "../platform/memory.h"
#include "../util.h"

namespace yar
{
Model::Model(std::shared_ptr<Renderer> renderer, std::string path) : m_path(path)
{
    const auto  full_path = fs_relative_path(path);
    const char* cpath     = full_path.c_str();
    LOG_DEBUG("Loading model {}", cpath);
    if (!fs_exists(full_path))
    {
        LOG_ERROR("No model found: {}", cpath);
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

    LOG_DEBUG("Loaded {} from model {}", Memory::Pretty(data->file_size), cpath);

    size_t totalIndexCount  = 0;
    size_t totalVertexCount = 0;

    for (size_t i = 0; i < data->meshes_count; i++)
    {
        for (size_t primIdx = 0; primIdx < data->meshes[i].primitives_count; primIdx++)
        {
            const auto&               primitive = data->meshes[i].primitives[primIdx];
            std::vector<Index>        indices   = {};
            std::vector<VertexShaded> vertices  = {};

            if (!ReadIndices(primitive, indices))
            {
                LOG_ERROR("Failed to read gltf indices: {}", cpath);
                cgltf_free(data);
                return;
            }

            if (!ReadVertices(primitive, vertices))
            {
                LOG_ERROR("Failed to read gltf vertices: {}", cpath);
                cgltf_free(data);
                return;
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

            std::shared_ptr<Material> material = ReadMaterial(renderer, primitive);

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

    UpdateAABB();
}

Model::~Model()
{
}

bool Model::FrustumCull(std::shared_ptr<Camera> camera)
{
    return !camera->IsInFrustum(m_aabb);
}

void Model::MarkAsCulled(std::shared_ptr<Renderer> renderer)
{
    for (const auto& mesh : m_meshes)
    {
        renderer->AddCulledMesh(mesh->GetVertexBuffer(), mesh->GetIndexBuffer());
    }
}

void Model::Render(std::shared_ptr<Renderer> renderer)
{
    for (const auto& mesh : m_meshes)
    {
        renderer->BindPipeline(RenderPipeline::SHADED);
        renderer->SetModelMatrix(m_transform);
        renderer->DrawWithBuffers(mesh->GetVertexBuffer(), mesh->GetIndexBuffer());
    }
}

void Model::RenderBounds(std::shared_ptr<Renderer> renderer)
{
    // TODO need some debug draw utils
}

void Model::UpdateAABB()
{
    m_aabb.min = {};
    m_aabb.max = {};

    for (const auto& mesh : m_meshes)
    {
        const auto min = m_transform.ToGlobalSpace(mesh->GetMin());
        const auto max = m_transform.ToGlobalSpace(mesh->GetMax());
        m_aabb.min     = glm::min(m_aabb.min, min);
        m_aabb.max     = glm::max(m_aabb.max, max);
    }
    LOG_DEBUG(
        "Model aabb min: [{:.2f}, {:.2f}, {:.2f}], max: [{:.2f}, {:.2f}, {:.2f}]",
        m_aabb.min.x,
        m_aabb.min.y,
        m_aabb.min.z,
        m_aabb.max.x,
        m_aabb.max.y,
        m_aabb.max.z
    );
}

bool Model::ReadIndices(const cgltf_primitive& primitive, std::vector<Index>& indices)
{
    const auto indexCount =
        cgltf_accessor_unpack_indices(primitive.indices, nullptr, sizeof(Index), 0);

    indices.resize(indexCount);

    auto readCount =
        cgltf_accessor_unpack_indices(primitive.indices, indices.data(), sizeof(Index), indexCount);

    return readCount == indexCount;
}

bool Model::ReadVertices(const cgltf_primitive& primitive, std::vector<VertexShaded>& vertices)
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
                    LOG_ERROR("Failed to read gltf normals: {}");
                    return false;
                }
                break;
            }

            case cgltf_attribute_type_normal:
            {
                if (!ReadFloats(primitive.attributes[attrIdx].data, vertNormals))
                {
                    LOG_ERROR("Failed to read gltf normals: {}");
                    return false;
                }
                break;
            }

            case cgltf_attribute_type_texcoord:
            {
                if (!ReadFloats(primitive.attributes[attrIdx].data, vertUVs))
                {
                    LOG_ERROR("Failed to read gltf UVs: {}");
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
        LOG_ERROR("gltf has no vertex positions: {}");
        return false;
    }
    if (vertNormals.size() <= 0)
    {
        LOG_ERROR("gltf has no vertex normals: {}");
        return false;
    }
    if (vertUVs.size() <= 0)
    {
        LOG_ERROR("gltf has no vertex UVs: {}");
        return false;
    }

    if (vertPositions.size() % 3 != 0)
    {
        LOG_ERROR("gltf vertex positions count not multiple of 3 ({})", vertPositions.size());
        return false;
    }
    if (vertNormals.size() != vertPositions.size())
    {
        LOG_ERROR(
            "gltf vertex normals invalid size ({}/{}): {}",
            vertNormals.size(),
            vertPositions.size()
        );
        return false;
    }
    if (vertUVs.size() != 2 * vertPositions.size() / 3)
    {
        LOG_ERROR("gltf vertex UVs invalid size ({}/{}): {}", vertUVs.size(), vertPositions.size());
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

bool Model::ReadFloats(cgltf_accessor* accessor, std::vector<float>& floats)
{
    const auto floatCount = cgltf_accessor_unpack_floats(accessor, nullptr, 0);
    floats.resize(floatCount);
    const auto readCount = cgltf_accessor_unpack_floats(accessor, floats.data(), floatCount);
    return readCount == floatCount;
}

std::shared_ptr<Material> Model::ReadMaterial(
    std::shared_ptr<Renderer> renderer,
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
        LOG_WARNING("Material has no name, rand: {}", name);
    };

    for (const auto& mat : m_materials)
    {
        if (mat->Name() == name)
        {
            return mat;
        }
    }

    LOG_DEBUG("Loading material {}", name);

    cgltf_texture_view* albedoView = nullptr;

    if (primitive.material->has_pbr_metallic_roughness)
    {
        albedoView = &primitive.material->pbr_metallic_roughness.base_color_texture;
    }
    else if (primitive.material->has_pbr_specular_glossiness)
    {
        albedoView = &primitive.material->pbr_specular_glossiness.diffuse_texture;
    }

    if (!albedoView)
    {
        LOG_ERROR("Material has no albedo view");
        return nullptr;
    }

    auto albedoTex = ReadTexture(renderer, albedoView);
    if (!albedoTex)
    {
        LOG_ERROR("Material has no albedo texture");
        return nullptr;
    }

    m_materials.push_back(std::make_shared<Material>(name, albedoTex));
    return m_materials.back();
}

std::shared_ptr<Texture> Model::ReadTexture(
    std::shared_ptr<Renderer> renderer,
    const cgltf_texture_view* view
)
{
    if (!view)
    {
        LOG_ERROR("Null texture view");
        return nullptr;
    }

    if (!view->texture)
    {
        LOG_ERROR("Texture view has no texture");
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
        LOG_WARNING("Texture has no name, rand: {}", name);
    }

    for (const auto& tex : m_textures)
    {
        if (tex->Name() == name)
        {
            return tex;
        }
    }

    LOG_DEBUG("Loading texture {}", name);

    const auto data =
        static_cast<const void*>(cgltf_buffer_view_data(view->texture->image->buffer_view));
    const auto size = static_cast<size_t>(view->texture->image->buffer_view->size);

    m_textures.push_back(std::make_shared<Texture>(renderer, name, size, data));
    return m_textures.back();
}
}; // namespace yar
