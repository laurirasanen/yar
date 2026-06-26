#pragma once

#include "texture.h"

#include <cstring>
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
        std::shared_ptr<Texture> orm,
        std::shared_ptr<Texture> normal,
        std::shared_ptr<Texture> emissive,
        float                    metalnessFactor,
        float                    roughnessFactor,
        float*                   emissiveFactor
    ) :
        m_name(name),
        m_albedo(albedo),
        m_orm(orm),
        m_normal(normal),
        m_emissive(emissive),
        m_metalnessFactor(metalnessFactor),
        m_roughnessFactor(roughnessFactor)
    {
        memcpy(m_emissiveFactor, emissiveFactor, sizeof(m_emissiveFactor));

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

    std::shared_ptr<Texture> GetORM()
    {
        return m_orm;
    }

    std::shared_ptr<Texture> GetNormal()
    {
        return m_normal;
    }

    std::shared_ptr<Texture> GetEmissive()
    {
        return m_emissive;
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

    float GetOcclusionFactor() const
    {
        return 2.0f;
    }

    const float* GetEmissiveFactor() const
    {
        return m_emissiveFactor;
    }

  private:
    std::string m_name;

    std::shared_ptr<Texture> m_albedo;
    std::shared_ptr<Texture> m_orm;
    std::shared_ptr<Texture> m_normal;
    std::shared_ptr<Texture> m_emissive;

    float m_metalnessFactor;
    float m_roughnessFactor;
    float m_emissiveFactor[3];

    MaterialQueue m_queue;
};
}; // namespace yar
