#pragma once

#include "transform.h"

#include <memory>

namespace yar
{
enum PhysicsBodyType
{
    BODY_STATIC,
    BODY_KINEMATIC,
    BODY_DYNAMIC,
};

enum PhysicsBodyShape
{
    SHAPE_BOX,
    SHAPE_SPHERE,
    SHAPE_CAPSULE,
};

struct PhysicsStats
{
    size_t   MemoryBytes;
    uint32_t Bodies;
    uint32_t Shapes;
    uint32_t Contacts;
    uint32_t ContactsAwake;
    uint32_t ContactsRecycled;
    uint32_t Joints;
    uint32_t Islands;
    uint32_t Tasks;
    double   UpdateTime;
};

class IPhysics
{
  public:
    IPhysics()          = default;
    virtual ~IPhysics() = default;

    IPhysics(const IPhysics&)            = delete;
    IPhysics(IPhysics&&)                 = delete;
    IPhysics& operator=(const IPhysics&) = delete;
    IPhysics& operator=(IPhysics&&)      = delete;

    virtual void Step() = 0;

    virtual std::shared_ptr<IPhysicsBody> CreateBody(
        PhysicsBodyType  type,
        PhysicsBodyShape shape,
        const glm::vec3& position,
        const glm::quat& rotation,
        const glm::vec3  extent
    ) = 0;

    virtual void DestroyBody(std::shared_ptr<IPhysicsBody> body) = 0;

    virtual void EnableBody(std::shared_ptr<IPhysicsBody> body) = 0;

    virtual void DisableBody(std::shared_ptr<IPhysicsBody> body) = 0;

    virtual TransformComponent GetTransform(std::shared_ptr<IPhysicsBody> body) = 0;

    virtual void SetTransform(std::shared_ptr<IPhysicsBody> body, const TransformComponent& t) = 0;

    virtual const PhysicsStats& GetStats() const = 0;
};

extern std::shared_ptr<IPhysics> g_physics;
}; // namespace yar
