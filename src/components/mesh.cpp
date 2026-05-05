#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "../log.h"
#include "../platform/fs.h"
#include "../platform/memory.h"
#include "mesh.h"

namespace yar
{
Mesh::Mesh(std::string path) : m_path(path)
{
    const auto  full_path = fs_relative_path(path);
    const char* cpath     = full_path.c_str();
    LOG_DEBUG("Getting mesh {}", cpath);
    if (!fs_exists(full_path))
    {
        LOG_ERROR("No mesh found: {}", cpath);
        return;
    }

    cgltf_options options {};
    cgltf_data*   data   = nullptr;
    cgltf_result  result = cgltf_parse_file(&options, cpath, &data);
    if (result != cgltf_result_success)
    {
        LOG_ERROR("Failed to parse mesh ({}): {}", static_cast<int>(result), cpath);
        cgltf_free(data);
        return;
    }

    LOG_DEBUG("Parsed {} from mesh {}", Memory::Pretty(data->file_size), cpath);

    // todo

    cgltf_free(data);
}

Mesh::~Mesh()
{
}
}; // namespace yar
