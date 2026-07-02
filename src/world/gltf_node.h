#pragma once

#include <memory>

#include "cgltf.h"

#include "../public/inode.h"
#include "../renderer/material.h"
#include "../renderer/mesh.h"
#include "../renderer/renderer.h"
#include "../renderer/texture.h"
#include "../ui/ui.h"

namespace yar
{
class GLTFNode : public INode
{
  public:
    GLTFNode() = delete;
    GLTFNode(std::shared_ptr<Renderer> renderer, std::string path);
    ~GLTFNode();

    GLTFNode(const GLTFNode&)            = delete;
    GLTFNode(GLTFNode&&)                 = delete;
    GLTFNode& operator=(const GLTFNode&) = delete;
    GLTFNode& operator=(GLTFNode&&)      = delete;

  private:
    bool ReadIndices(const cgltf_primitive& primitive, std::vector<Index>& indices);
    bool ReadVertices(const cgltf_primitive& primitive, std::vector<VertexShaded>& vertices);
    bool ReadFloats(cgltf_accessor* accessor, std::vector<float>& floats);

    std::shared_ptr<Material> ReadMaterial(
        std::shared_ptr<Renderer> renderer,
        const cgltf_primitive&    primitive
    );

    std::shared_ptr<Texture> ReadTexture(
        std::shared_ptr<Renderer> renderer,
        const cgltf_texture_view* view,
        TextureType               type
    );

    std::string m_path;

    std::vector<std::shared_ptr<Mesh<VertexShaded>>> m_meshes;
    std::vector<std::shared_ptr<Material>>           m_materials;
    std::vector<std::shared_ptr<Texture>>            m_textures;
};
}; // namespace yar
