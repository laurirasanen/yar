#pragma once

#include "../public/engine/iphysics.h"

#include "box3d/box3d.h"

namespace yar
{
class PhysicsShape final : public IPhysicsShape
{
  public:
    PhysicsShape() = delete;

    explicit PhysicsShape(b3ShapeId id) : m_shape(id)
    {
    }

    b3ShapeId GetShape() const
    {
        return m_shape;
    }

  private:
    b3ShapeId m_shape;
};

class PhysicsBody final : public IPhysicsBody
{
  public:
    PhysicsBody() = delete;

    explicit PhysicsBody(b3BodyId bodyId) : m_bodyId(bodyId)
    {
    }

    ~PhysicsBody()
    {
    }

    b3BodyId GetBodyID() const
    {
        return m_bodyId;
    }

    void AddShape(PhysicsShape shape)
    {
        m_shapes.push_back(shape);
    }

    void SetShapes(const std::vector<PhysicsShape>& shapes)
    {
        m_shapes = shapes;
    }

    std::vector<PhysicsShape> GetShapes() const
    {
        return m_shapes;
    }

  private:
    b3BodyId                  m_bodyId;
    std::vector<PhysicsShape> m_shapes;
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

    void Step(float deltaTime) override;

    void AddShape(
        std::shared_ptr<IPhysicsBody> body,
        PhysicsShapeType              type,
        const glm::vec3&              position,
        const glm::quat&              rotation,
        const glm::vec3&              extent
    ) override;

    std::shared_ptr<IPhysicsBody> CreateBody(
        PhysicsBodyType  type,
        const glm::vec3& position,
        const glm::quat& rotation
    ) override;

    void DestroyBody(std::shared_ptr<IPhysicsBody> body) override;

    void EnableBody(std::shared_ptr<IPhysicsBody> body) override;

    void DisableBody(std::shared_ptr<IPhysicsBody> body) override;

    Transform GetTransform(std::shared_ptr<IPhysicsBody> body) override;

    void SetTransform(std::shared_ptr<IPhysicsBody> body, const Transform& t) override;

    const PhysicsStats& GetStats() const override
    {
        return m_stats;
    }

  private:
    b3WorldId    m_worldId;
    PhysicsStats m_stats;
};
}; // namespace yar
