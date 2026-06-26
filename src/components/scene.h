#pragma once

#include <memory>

#include "cgltf.h"

#include "../renderer/renderer.h"
#include "../ui/ui.h"
#include "material.h"
#include "mesh.h"
#include "texture.h"

namespace yar
{
class Scene
{
  public:
    Scene() = delete;
    Scene(std::shared_ptr<Renderer> renderer, std::shared_ptr<UI> ui, std::string path);
    ~Scene();

    Scene(const Scene&)            = delete;
    Scene(Scene&&)                 = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&)      = delete;

    std::vector<std::shared_ptr<Mesh<VertexShaded>>> GetMeshes()
    {
        return m_meshes;
    }

    void SetTransform(const Transform& trans)
    {
        for (const auto& mesh : m_meshes)
        {
            mesh->GetTransform()->CopyFrom(trans);
            mesh->UpdateAABB();
        }
    }

  private:
    bool ReadIndices(const cgltf_primitive& primitive, std::vector<Index>& indices);
    bool ReadVertices(const cgltf_primitive& primitive, std::vector<VertexShaded>& vertices);
    bool ReadFloats(cgltf_accessor* accessor, std::vector<float>& floats);

    std::shared_ptr<Material> ReadMaterial(
        std::shared_ptr<Renderer> renderer,
        std::shared_ptr<UI>       ui,
        const cgltf_primitive&    primitive
    );

    std::shared_ptr<Texture> ReadTexture(
        std::shared_ptr<Renderer> renderer,
        std::shared_ptr<UI>       ui,
        const cgltf_texture_view* view,
        TextureType               type
    );

    std::string m_path;

    std::vector<std::shared_ptr<Mesh<VertexShaded>>> m_meshes;
    std::vector<std::shared_ptr<Material>>           m_materials;
    std::vector<std::shared_ptr<Texture>>            m_textures;
};
}; // namespace yar
