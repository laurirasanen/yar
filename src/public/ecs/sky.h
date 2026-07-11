#pragma once

#include "../platform/fs.h"
#include "../resource/resource.h"
#include "../resource/texture.h"
#include "component.h"

#include <string>

namespace yar
{
class SkyComponent : public Component
{
  public:
    SkyComponent(Entity* owner, const std::string& path) : Component(owner), m_path(path)
    {
    }

    const ResourceHandle<Texture>& GetColor() const
    {
        return m_color;
    }

    const ResourceHandle<Texture>& GetDiffuse() const
    {
        return m_diffuse;
    }

    const ResourceHandle<Texture>& GetSpecular() const
    {
        return m_specular;
    }

    const ResourceHandle<Texture>& GetLUT() const
    {
        return m_lut;
    }

  protected:
    void OnInitialize() override
    {
        const auto f = fs_relative_path(m_path);

        const auto iblColorPath    = fs_append(f, "color.hdr");
        const auto iblLUTPath      = fs_append(f, "lut.png");
        const auto iblDiffusePath  = fs_append(f, "diffuse.ktx2");
        const auto iblSpecularPath = fs_append(f, "specular.ktx2");

        m_color    = g_resources->Load<Texture>(iblColorPath, TextureType::TEX_IBL);
        m_diffuse  = g_resources->Load<Texture>(iblDiffusePath, TextureType::TEX_IBL);
        m_specular = g_resources->Load<Texture>(iblSpecularPath, TextureType::TEX_IBL);
        m_lut      = g_resources->Load<Texture>(iblLUTPath, TextureType::TEX_IBL_LUT);
    }

    void OnDestroy() override
    {
    }

    void Render() override
    {
    }

  private:
    std::string m_path;

    ResourceHandle<Texture> m_color;
    ResourceHandle<Texture> m_diffuse;
    ResourceHandle<Texture> m_specular;
    ResourceHandle<Texture> m_lut;
};
}; // namespace yar
