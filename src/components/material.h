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

    Material(
        std::string              name,
        std::shared_ptr<Texture> albedo,
        std::shared_ptr<Texture> normal,
        std::shared_ptr<Texture> mrao,
        float                    metalnessFactor,
        float                    roughnessFactor
    ) :
        m_name(name),
        m_albedo(albedo),
        m_normal(normal),
        m_mrao(mrao),
        m_metalnessFactor(metalnessFactor),
        m_roughnessFactor(roughnessFactor)
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

    std::shared_ptr<Texture> GetNormal()
    {
        return m_normal;
    }

    std::shared_ptr<Texture> GetMRAO()
    {
        return m_mrao;
    }

    MaterialQueue GetQueue() const
    {
        return m_queue;
    }

    float GetMetalnessFactor() const
    {
        return m_metalnessFactor;
    }

    float GetRoughnessFactor() const
    {
        return m_roughnessFactor;
    }

  private:
    std::string m_name;

    std::shared_ptr<Texture> m_albedo;
    std::shared_ptr<Texture> m_normal;
    std::shared_ptr<Texture> m_mrao;

    float m_metalnessFactor;
    float m_roughnessFactor;

    MaterialQueue m_queue;
};
}; // namespace yar
