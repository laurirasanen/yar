#pragma once

#include "../engine/iphysics.h"
#include "component.h"

namespace yar
{
class ColliderComponent : public Component
{
  public:
    ColliderComponent(Entity* owner, PhysicsShapeType shape, glm::vec3 size) :
        Component(owner),
        m_shape(shape),
        m_size(size)
    {
    }

    PhysicsShapeType GetShapeType() const
    {
        return m_shape;
    }

    const glm::vec3& GetSize() const
    {
        return m_size;
    }

  private:
    PhysicsShapeType m_shape;
    glm::vec3        m_size;
};
}; // namespace yar
