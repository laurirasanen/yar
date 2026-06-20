#pragma once

#include <memory>

#include "../renderer/renderer.h"
#include "material.h"
#include "mesh.h"

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
    std::string m_path;

    std::vector<std::shared_ptr<Mesh<VertexUnlit>>> m_meshes;
    std::vector<std::shared_ptr<Material>>          m_materials;
};
}; // namespace yar
