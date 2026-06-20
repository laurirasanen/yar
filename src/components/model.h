#pragma once

#include <memory>

#include "../renderer/renderer.h"
#include "cgltf.h"
#include "material.h"
#include "mesh.h"
#include "texture.h"

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

    void Render(std::shared_ptr<Renderer> renderer);

  private:
    bool ReadFloats(cgltf_accessor* accessor, std::vector<float>& floats);

    std::string m_path;

    std::vector<std::shared_ptr<Mesh<VertexShaded>>> m_meshes;
    std::vector<std::shared_ptr<Material>>           m_materials;
    std::vector<std::shared_ptr<Texture>>            m_textures;
};
}; // namespace yar
