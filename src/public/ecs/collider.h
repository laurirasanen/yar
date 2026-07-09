#pragma once

#include "../engine/iphysics.h"
#include "component.h"

namespace yar
{
class ColliderComponent : public Component
{
  public:
    ColliderComponent(PhysicsShapeType shape, glm::vec3 size) :
        Component(),
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
