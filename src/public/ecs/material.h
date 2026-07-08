#pragma once

#include "../resource/texture.h"
#include "component.h"

#include <vector>

namespace yar
{
enum MaterialQueue
{
    QUEUE_OPAQUE,
    QUEUE_TRANSPARENT,
    QUEUE_MAX
};

class MaterialComponent : public Component
{
  public:
    void SetQueue(MaterialQueue queue)
    {
        m_queue = queue;
    }

    MaterialQueue GetQueue() const
    {
        return m_queue;
    }

    void SetTextures(const std::vector<Texture>& tex)
    {
        m_textures = tex;
    }

    const std::vector<Texture>& GetTextures() const
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
    MaterialQueue m_queue;

    std::vector<Texture> m_textures;
    std::vector<float>   m_parameters;
};
}; // namespace yar
