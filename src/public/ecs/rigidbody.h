#pragma once

#include "collider.h"
#include "component.h"

namespace yar
{
class RigidBodyComponent : public Component
{
  private:
    ColliderComponent m_collider;
};
}; // namespace yar
