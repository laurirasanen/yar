#pragma once

namespace yar
{
class Material
{
  public:
    Material() = delete;
    Material(std::string path);
    ~Material();

    Material(const Material&)            = delete;
    Material(Material&&)                 = delete;
    Material& operator=(const Material&) = delete;
    Material& operator=(Material&&)      = delete;

  private:
    std::string m_path;
};
}; // namespace yar
