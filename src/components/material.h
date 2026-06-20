#pragma once

#include <string>

namespace yar
{
class Material
{
  public:
    Material() = delete;
    Material(std::string name);
    ~Material();

    Material(const Material&)            = delete;
    Material(Material&&)                 = delete;
    Material& operator=(const Material&) = delete;
    Material& operator=(Material&&)      = delete;

  private:
    std::string m_name;
};
}; // namespace yar
