#pragma once

#include "texture.h"

#include <memory>
#include <string>

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
    Material() = delete;

    Material(std::string name, std::shared_ptr<Texture> albedo) : m_name(name), m_albedo(albedo)
    {
        if (albedo->HasTransparency())
        {
            m_queue = MaterialQueue::QUEUE_TRANSPARENT;
        }
        else
        {
            m_queue = MaterialQueue::QUEUE_OPAQUE;
        }
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

    std::shared_ptr<Texture> GetAlbedo()
    {
        return m_albedo;
    }

    MaterialQueue GetQueue() const
    {
        return m_queue;
    }

  private:
    std::string m_name;

    std::shared_ptr<Texture> m_albedo;

    MaterialQueue m_queue;
};
}; // namespace yar
