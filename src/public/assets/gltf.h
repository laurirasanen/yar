#pragma once

#include <cgltf.h>

#include "../resource/resource.h"
#include "../resource/texture.h"

#include <string>
#include <vector>

namespace yar
{
struct GltfTextures
{
    ResourceHandle<Texture> albedo;
    ResourceHandle<Texture> orm;
    ResourceHandle<Texture> normal;
    ResourceHandle<Texture> emissive;
};

struct GltfParameters
{
    float metallic;
    float roughness;
    float emissive[3];
};

struct GltfData
{
    std::vector<uint32_t> indices;
    std::vector<float>    positions;
    std::vector<float>    normals;
    std::vector<float>    tangents;
    std::vector<float>    uvs;
    GltfTextures          textures;
    GltfParameters        parameters;
};

class GLTF
{
  public:
    GLTF()                       = delete;
    ~GLTF()                      = delete;
    GLTF(const GLTF&)            = delete;
    GLTF(GLTF&&)                 = delete;
    GLTF& operator=(const GLTF&) = delete;
    GLTF& operator=(GLTF&&)      = delete;

    bool Load(const std::string& path, std::vector<GltfData>& data);

  private:
    bool ReadIndices(const cgltf_primitive& primitive, GltfData& data);

    bool ReadVertices(const cgltf_primitive& primitive, GltfData& data);
    bool ReadFloats(cgltf_accessor* accessor, std::vector<float>& floats);

    void CalculateTangents(GltfData& data);

    bool                    ReadTextures(const cgltf_primitive& primitive, GltfData& data);
    ResourceHandle<Texture> ReadTexture(const cgltf_texture_view* view, TextureType type);
};
}; // namespace yar
