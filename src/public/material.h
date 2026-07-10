#pragma once

#include "ecs/component.h"
#include "resource/shader.h"
#include "resource/texture.h"

#include <vector>

namespace yar
{
class Material
{
  public:
    Material(ResourceHandle<Shader> vert, ResourceHandle<Shader> frag) : m_vert(vert), m_frag(frag)
    {
    }

    void SetTextures(const std::vector<ResourceHandle<Texture>>& tex)
    {
        m_textures = tex;
    }

    const std::vector<ResourceHandle<Texture>>& GetTextures() const
    {
        return m_textures;
    }

    void SetParameters(const std::vector<float>& params)
    {
        m_parameters = params;
    }

    const std::vector<float>& GetParameters() const
    {
        return m_parameters;
    }

  private:
    ResourceHandle<Shader> m_vert;
    ResourceHandle<Shader> m_frag;

    std::vector<ResourceHandle<Texture>> m_textures;
    std::vector<float>                   m_parameters;
};
}; // namespace yar
