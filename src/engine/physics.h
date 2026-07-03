#pragma once

#include "../public/iphysics.h"

#include "box3d/box3d.h"

namespace yar
{
class PhysicsBody : public IPhysicsBody
{
  public:
    PhysicsBody() = delete;

    PhysicsBody(b3BodyId bodyId, b3ShapeId shapeId) : m_bodyId(bodyId), m_shapeId(shapeId)
    {
    }

    ~PhysicsBody()
    {
    }

    b3BodyId GetBodyID() const
    {
        return m_bodyId;
    }

    b3ShapeId GetShapeID() const
    {
        return m_shapeId;
    }

  private:
    b3BodyId  m_bodyId;
    b3ShapeId m_shapeId;
};

class Physics : public IPhysics
{
  public:
    Physics();
    ~Physics();

    Physics(const Physics&)            = delete;
    Physics(Physics&&)                 = delete;
    Physics& operator=(const Physics&) = delete;
    Physics& operator=(Physics&&)      = delete;

    void Step() override;

    std::shared_ptr<IPhysicsBody> CreateBody(
        PhysicsBodyType  type,
        PhysicsBodyShape shape,
        const glm::vec3& position,
        const glm::quat& rotation,
        const glm::vec3  extent
    ) override;

    void DestroyBody(std::shared_ptr<IPhysicsBody> body) override;

    void EnableBody(std::shared_ptr<IPhysicsBody> body) override;

    void DisableBody(std::shared_ptr<IPhysicsBody> body) override;

    Transform GetTransform(std::shared_ptr<IPhysicsBody> body) override;

    void SetTransform(std::shared_ptr<IPhysicsBody> body, const Transform& t) override;

    size_t MemoryUsage() override;

  private:
    b3WorldId m_worldId;
};
}; // namespace yar
