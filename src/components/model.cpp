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
    LOG_DEBUG("Getting model {}", cpath);
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
        for (size_t prim = 0; prim < data->meshes[i].primitives_count; prim++)
        {
            std::vector<Index> indices    = {};
            const auto         indexCount = data->meshes[i].primitives[prim].indices->count;
            indices.reserve(indexCount);

            const auto readCount = cgltf_accessor_unpack_indices(
                data->meshes[i].primitives[prim].indices,
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

            LOG_DEBUG("Parsed {} indices from primitive {}", indexCount, prim);

            std::vector<VertexUnlit> vertices = {};
            // TODO

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

        LOG_DEBUG("Parsed {} primitives from mesh {}", data->meshes[i].primitives_count, i);
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
