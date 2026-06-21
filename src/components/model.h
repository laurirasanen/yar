#pragma once

#include <memory>

#include "../renderer/renderer.h"
#include "cgltf.h"
#include "geometry.h"
#include "material.h"
#include "mesh.h"
#include "texture.h"
#include "transform.h"

namespace yar
{
class Model
{
  public:
    Model() = delete;
    Model(std::shared_ptr<Renderer> renderer, std::string path);
    ~Model();

    Model(const Model&)            = delete;
    Model(Model&&)                 = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&)      = delete;

    bool FrustumCull(std::shared_ptr<Camera> camera);

    void MarkAsCulled(std::shared_ptr<Renderer> renderer);

    void Render(std::shared_ptr<Renderer> renderer);

    void RenderBounds(std::shared_ptr<Renderer> renderer);

    Transform& GetTransform()
    {
        return m_transform;
    }

    void UpdateAABB();

  private:
    bool ReadFloats(cgltf_accessor* accessor, std::vector<float>& floats);

    std::string m_path;

    std::vector<std::shared_ptr<Mesh<VertexShaded>>> m_meshes;
    std::vector<std::shared_ptr<Material>>           m_materials;
    std::vector<std::shared_ptr<Texture>>            m_textures;

    Transform m_transform;
    AABB      m_aabb;
};
}; // namespace yar
