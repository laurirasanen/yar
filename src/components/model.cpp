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
            const auto& primitive = data->meshes[i].primitives[primIdx];

            std::vector<Index> indices = {};

            const auto indexCount =
                cgltf_accessor_unpack_indices(primitive.indices, nullptr, sizeof(Index), 0);

            indices.resize(indexCount);

            auto readCount = cgltf_accessor_unpack_indices(
                primitive.indices,
                indices.data(),
                sizeof(Index),
                indexCount
            );

            if (readCount != indexCount)
            {
                LOG_ERROR(
                    "Failed to read gltf indices ({} != {}): {}",
                    readCount,
                    indexCount,
                    cpath
                );
                cgltf_free(data);
                return;
            }

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
                            LOG_ERROR("Failed to read gltf normals: {}", cpath);
                            cgltf_free(data);
                            return;
                        }
                        break;
                    }

                    case cgltf_attribute_type_normal:
                    {
                        if (!ReadFloats(primitive.attributes[attrIdx].data, vertNormals))
                        {
                            LOG_ERROR("Failed to read gltf normals: {}", cpath);
                            cgltf_free(data);
                            return;
                        }
                        break;
                    }

                    case cgltf_attribute_type_texcoord:
                    {
                        if (!ReadFloats(primitive.attributes[attrIdx].data, vertUVs))
                        {
                            LOG_ERROR("Failed to read gltf UVs: {}", cpath);
                            cgltf_free(data);
                            return;
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
                LOG_ERROR("gltf has no vertex positions: {}", cpath);
                cgltf_free(data);
                return;
            }
            if (vertNormals.size() <= 0)
            {
                LOG_ERROR("gltf has no vertex normals: {}", cpath);
                cgltf_free(data);
                return;
            }
            if (vertUVs.size() <= 0)
            {
                LOG_ERROR("gltf has no vertex UVs: {}", cpath);
                cgltf_free(data);
                return;
            }

            if (vertPositions.size() % 3 != 0)
            {
                LOG_ERROR(
                    "gltf vertex positions count not multiple of 3 ({}): {}",
                    vertPositions.size(),
                    cpath
                );
                cgltf_free(data);
                return;
            }
            if (vertNormals.size() != vertPositions.size())
            {
                LOG_ERROR(
                    "gltf vertex normals invalid size ({}/{}): {}",
                    vertNormals.size(),
                    vertPositions.size(),
                    cpath
                );
                cgltf_free(data);
                return;
            }
            if (vertUVs.size() != 2 * vertPositions.size() / 3)
            {
                LOG_ERROR(
                    "gltf vertex UVs invalid size ({}/{}): {}",
                    vertUVs.size(),
                    vertPositions.size(),
                    cpath
                );
                cgltf_free(data);
                return;
            }

            const size_t vertexCount = vertPositions.size() / 3;

            std::vector<VertexShaded> vertices = {};
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

            std::shared_ptr<Buffer> vertexBuffer;
            renderer->CreateBuffer(
                vertexBuffer,
                VertexBuffer,
                vertices.data(),
                sizeof(VertexUnlit),
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

            auto mesh =
                std::make_shared<Mesh<VertexShaded>>(vertices, indices, vertexBuffer, indexBuffer);
            m_meshes.push_back(mesh);

            totalIndexCount += indexCount;
            totalVertexCount += vertexCount;
        }
    }

    LOG_DEBUG(
        "Parsed {} meshes, {} verts, {} indices from {}",
        m_meshes.size(),
        totalVertexCount,
        totalIndexCount,
        cpath
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

bool Model::ReadFloats(cgltf_accessor* accessor, std::vector<float>& floats)
{
    const auto floatCount = cgltf_accessor_unpack_floats(accessor, nullptr, 0);
    floats.resize(floatCount);
    const auto readCount = cgltf_accessor_unpack_floats(accessor, floats.data(), floatCount);
    return readCount == floatCount;
}

void Model::UpdateAABB()
{
    m_aabb.min = {};
    m_aabb.max = {};

    for (const auto& mesh : m_meshes)
    {
        const auto min = m_transform.ToGlobalSpace(mesh->GetMin());
        const auto max = m_transform.ToGlobalSpace(mesh->GetMax());
        // clang-format off
        if (min.x < m_aabb.min.x) m_aabb.min.x = min.x;
        if (min.y < m_aabb.min.y) m_aabb.min.y = min.y;
        if (min.z < m_aabb.min.z) m_aabb.min.z = min.z;
        if (max.x > m_aabb.max.x) m_aabb.max.x = max.x;
        if (max.y > m_aabb.max.y) m_aabb.max.y = max.y;
        if (max.z > m_aabb.max.z) m_aabb.max.z = max.z;
        // clang-format on
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
}; // namespace yar
