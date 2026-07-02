#pragma once

#include "../public/ibuffer.h"
#include "../public/isky.h"
#include "../renderer/texture.h"

#include <memory>

namespace yar
{
class Sky : public ISky
{
  public:
    Sky() = delete;

    Sky(const Sky&)            = delete;
    Sky(Sky&&)                 = delete;
    Sky& operator=(const Sky&) = delete;
    Sky& operator=(Sky&&)      = delete;

    Sky(std::string folder);
    ~Sky() {};

    std::shared_ptr<ITexture> GetColor() override
    {
        return m_color;
    }

    std::shared_ptr<ITexture> GetLUT() override
    {
        return m_lut;
    }

    std::shared_ptr<ITexture> GetDiffuse() override
    {
        return m_diffuse;
    }

    std::shared_ptr<ITexture> GetSpecular() override
    {
        return m_specular;
    }

    void Render() override;

  private:
    std::shared_ptr<Texture> m_color;
    std::shared_ptr<Texture> m_lut;
    std::shared_ptr<Texture> m_diffuse;
    std::shared_ptr<Texture> m_specular;

    std::shared_ptr<IBuffer> m_vertexBuffer;
    std::shared_ptr<IBuffer> m_indexBuffer;
};
}; // namespace yar
