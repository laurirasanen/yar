#pragma once

#include "ecs/component.h"
#include "resource/shader.h"
#include "resource/texture.h"

#include <vector>

namespace yar
{
enum MaterialQueue
{
    QUEUE_OPAQUE,
    QUEUE_TRANSPARENT,
    QUEUE_MAX
};

class Material
{
  public:
    Material(ResourceHandle<Shader> vert, ResourceHandle<Shader> frag, MaterialQueue queue) :
        m_vert(vert),
        m_frag(frag),
        m_queue(queue)
    {
#error todo
    }

    MaterialQueue GetQueue() const
    {
        return m_queue;
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
    MaterialQueue          m_queue;

    std::vector<ResourceHandle<Texture>> m_textures;
    std::vector<float>                   m_parameters;
};
}; // namespace yar
