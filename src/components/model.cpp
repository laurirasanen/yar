#include "src/renderer/renderer.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "../log.h"
#include "../platform/fs.h"
#include "../platform/memory.h"
#include "model.h"

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

    for (size_t i = 0; i < data->meshes_count; i++)
    {
        for (size_t primIdx = 0; primIdx < data->meshes[i].primitives_count; primIdx++)
        {
            const auto& primitive = data->meshes[i].primitives[primIdx];

            std::vector<Index>       indices  = {};
            std::vector<VertexUnlit> vertices = {};

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
            for (size_t attrIdx = 0; attrIdx < primitive.attributes_count; attrIdx++)
            {
                switch (primitive.attributes[attrIdx].type)
                {
                    case cgltf_attribute_type_position:
                    {
                        const auto vertCount = cgltf_accessor_unpack_floats(
                            primitive.attributes[attrIdx].data,
                            nullptr,
                            0
                        );

                        vertPositions.resize(vertCount);

                        readCount = cgltf_accessor_unpack_floats(
                            primitive.attributes[attrIdx].data,
                            vertPositions.data(),
                            vertCount
                        );

                        if (readCount != vertCount)
                        {
                            LOG_ERROR(
                                "Failed to read gltf vertices ({} != {}): {}",
                                readCount,
                                indexCount,
                                cpath
                            );
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

            const size_t vertexCount = vertPositions.size() / 3;
            vertices.resize(vertexCount);

            for (size_t vertIdx = 0; vertIdx < vertexCount; vertIdx++)
            {
                vertices[vertIdx].position.x = vertPositions[vertIdx * 3 + 0];
                vertices[vertIdx].position.y = vertPositions[vertIdx * 3 + 1];
                vertices[vertIdx].position.z = vertPositions[vertIdx * 3 + 2];

                vertices[vertIdx].color.r = 0.5f;
                vertices[vertIdx].color.g = 0.5f;
                vertices[vertIdx].color.b = 0.5f;
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
                std::make_shared<Mesh<VertexUnlit>>(vertices, indices, vertexBuffer, indexBuffer);
            m_meshes.push_back(mesh);
        }
    }

    cgltf_free(data);
}

Model::~Model()
{
}

void Model::Render(std::shared_ptr<Renderer> renderer)
{
    for (const auto& mesh : m_meshes)
    {
        renderer->BindPipeline(RenderPipeline::TEST);
        renderer->DrawWithBuffers(mesh->GetVertexBuffer(), mesh->GetIndexBuffer());
    }
}
}; // namespace yar
