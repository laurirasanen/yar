#pragma once

#include "resource/shader.h"
#include "resource/texture.h"

#include <vector>

namespace yar
{
class Material
{
  public:
    Material()
    {
    }

    Material(ResourceHandle<Shader> shader) : m_shader(shader)
    {
    }

    const ResourceHandle<Shader> GetShader() const
    {
        return m_shader;
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
    ResourceHandle<Shader> m_shader;

    std::vector<ResourceHandle<Texture>> m_textures;
    std::vector<float>                   m_parameters;
};
}; // namespace yar
