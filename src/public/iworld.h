#pragma once

#include "inode.h"
#include "isky.h"

#include <memory>

namespace yar
{
class IWorld
{
  public:
    IWorld() : m_enabled(false)
    {
    }

    virtual ~IWorld() = default;

    IWorld(const IWorld&)            = delete;
    IWorld(IWorld&&)                 = delete;
    IWorld& operator=(const IWorld&) = delete;
    IWorld& operator=(IWorld&&)      = delete;

    virtual void AddNode(std::shared_ptr<INode> node) = 0;

    virtual void SetSky(std::shared_ptr<ISky> sky) = 0;

    virtual void Frame()  = 0;
    virtual void Tick()   = 0;
    virtual void Render() = 0;

    void SetEnabled(bool enabled)
    {
        m_enabled = enabled;
    }

  protected:
    bool m_enabled;
};

extern std::shared_ptr<IWorld> g_world;
}; // namespace yar
