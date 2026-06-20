#pragma once

#include <string>

namespace yar
{
class Texture
{
  public:
    Texture() = delete;
    Texture(std::string path);
    ~Texture();

    Texture(const Texture&)            = delete;
    Texture(Texture&&)                 = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&)      = delete;

  private:
    std::string m_path;
};
}; // namespace yar
