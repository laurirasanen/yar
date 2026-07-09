#pragma once

#include "../transform.h"
#include "component.h"

namespace yar
{
class TransformComponent : public Component
{
  public:
    Transform* GetTransform()
    {
        return &m_transform;
    }

  private:
    Transform m_transform;
};
} // namespace yar
