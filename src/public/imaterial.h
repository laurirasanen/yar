#pragma once

#include <string>

namespace yar
{
enum MaterialQueue
{
    QUEUE_OPAQUE,
    QUEUE_TRANSPARENT,
    QUEUE_MAX
};

class IMaterial
{
  public:
    IMaterial()          = delete;
    virtual ~IMaterial() = default;

    IMaterial(std::string name) : m_name(name) {};

    IMaterial(const IMaterial&)            = delete;
    IMaterial(IMaterial&&)                 = delete;
    IMaterial& operator=(const IMaterial&) = delete;
    IMaterial& operator=(IMaterial&&)      = delete;

    std::string Name() const
    {
        return m_name;
    }

    MaterialQueue GetQueue() const
    {
        return m_queue;
    }

  protected:
    std::string m_name;

    MaterialQueue m_queue;
};
}; // namespace yar
