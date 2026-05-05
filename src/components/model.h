#pragma once

#include <memory>

#include "material.h"
#include "mesh.h"

namespace yar
{
class Model
{
  public:
    Model() = delete;
    Model(std::string path);
    ~Model();

    Model(const Model&)            = delete;
    Model(Model&&)                 = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&)      = delete;

  private:
    std::string m_path;

    std::shared_ptr<Mesh>     m_mesh;
    std::shared_ptr<Material> m_material;
};
}; // namespace yar
