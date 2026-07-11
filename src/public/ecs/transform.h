#pragma once

#include "../transform.h"
#include "component.h"

namespace yar
{
class TransformComponent : public Component
{
  public:
    TransformComponent(Entity* owner) : Component(owner)
    {
    }

    Transform* GetTransform()
    {
        return &m_transform;
    }

  private:
    Transform m_transform;
};
} // namespace yar
