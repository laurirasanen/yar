#pragma once

#include "texture.h"

#include <memory>
#include <string>

namespace yar
{
class Material
{
  public:
    Material() = delete;

    Material(std::string name, std::shared_ptr<Texture> albedo) : m_name(name), m_albedo(albedo)
    {
    }

    ~Material()
    {
    }

    Material(const Material&)            = delete;
    Material(Material&&)                 = delete;
    Material& operator=(const Material&) = delete;
    Material& operator=(Material&&)      = delete;

    std::string Name() const
    {
        return m_name;
    }

  private:
    std::string m_name;

    std::shared_ptr<Texture> m_albedo;
};
}; // namespace yar
