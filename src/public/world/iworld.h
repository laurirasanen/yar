#pragma once

#include "../ecs/entity.h"

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

    virtual void AddEntity(std::shared_ptr<Entity> entity) = 0;

    virtual void Update(float deltaTime)      = 0;
    virtual void FixedUpdate(float deltaTime) = 0;
    virtual void Render()                     = 0;

    void SetEnabled(bool enabled)
    {
        m_enabled = enabled;
    }

  protected:
    bool m_enabled;
};

extern std::shared_ptr<IWorld> g_world;
}; // namespace yar
