#include "physics.h"

#include "../public/time_util.h"
#include "../public/util.h"
#include "box3d/box3d.h"
#include "box3d/id.h"
#include "src/public/iphysics.h"

#include <thread>

namespace yar
{
constexpr std::shared_ptr<PhysicsBody> InternalBody(std::shared_ptr<IPhysicsBody> body)
{
    const auto b = dynamic_pointer_cast<PhysicsBody>(body);
    if (b == nullptr)
    {
        throw std::runtime_error("Invalid type for physics body");
    }
    return b;
}

constexpr b3BodyType BodyTypeToB3(PhysicsBodyType t)
{
    switch (t)
    {
        case PhysicsBodyType::BODY_STATIC:
        {
            return b3_staticBody;
        }

        case PhysicsBodyType::BODY_KINEMATIC:
        {
            return b3_kinematicBody;
        }

        case PhysicsBodyType::BODY_DYNAMIC:
        {
            return b3_dynamicBody;
        }

        default:
        {
            throw std::runtime_error(std::format("Unhandled body type {}", static_cast<int>(t)));
        }
    }
}

constexpr b3Pos GlmToB3(const glm::vec3& v)
{
    return {v.x, v.y, v.z};
}

constexpr b3Quat GlmToB3(const glm::quat& q)
{
    return {
        .v = {q.x, q.y, q.z},
        .s = q.w
    };
}

constexpr glm::vec3 B3ToGlm(const b3Pos& v)
{
    return {v.x, v.y, v.z};
}

constexpr glm::quat B3ToGlm(const b3Quat& q)
{
    glm::quat g = {};

    g.x = q.v.x;
    g.y = q.v.y;
    g.z = q.v.z;
    g.w = q.s;

    return g;
}

Physics::Physics()
{
    auto worldDef    = b3DefaultWorldDef();
    worldDef.gravity = {0, -9.81f, 0};

    uint32_t cores       = std::thread::hardware_concurrency();
    cores                = CLAMP(cores, 1, 8);
    worldDef.workerCount = cores;

    // Initial capacity to avoid allocs at runtime.
    // TODO: adjust
    worldDef.capacity.contactCount      = 1024;
    worldDef.capacity.dynamicBodyCount  = 64;
    worldDef.capacity.dynamicShapeCount = 64;
    worldDef.capacity.staticBodyCount   = 256;
    worldDef.capacity.staticShapeCount  = 256;

    m_worldId = b3CreateWorld(&worldDef);
}

Physics::~Physics()
{
    b3DestroyWorld(m_worldId);
}

void Physics::Step()
{
    const int   subStepCount = 4;
    const float timeStep     = static_cast<float>(Time::DeltaTick);
    b3World_Step(m_worldId, timeStep, subStepCount);
}

std::shared_ptr<IPhysicsBody> Physics::CreateBody(
    PhysicsBodyType  type,
    PhysicsBodyShape shape,
    const glm::vec3& position,
    const glm::quat& rotation,
    const glm::vec3  extent
)
{
    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.type      = BodyTypeToB3(type);
    bodyDef.position  = GlmToB3(position);
    bodyDef.rotation  = GlmToB3(rotation);
    bodyDef.isEnabled = false;

    b3BodyId bodyId = b3CreateBody(m_worldId, &bodyDef);

    b3ShapeDef shapeDef = b3DefaultShapeDef();
    b3ShapeId  shapeId  = {};

    switch (shape)
    {
        case PhysicsBodyShape::SHAPE_BOX:
        {
            b3BoxHull hull = b3MakeBoxHull(extent.x, extent.y, extent.z);
            shapeId        = b3CreateHullShape(bodyId, &shapeDef, &hull.base);
            break;
        }

        case PhysicsBodyShape::SHAPE_SPHERE:
        {
            const auto longestAxis = MAX(MAX(extent.x, extent.y), extent.z);
            b3Sphere   sphere      = {.center = {}, .radius = longestAxis};
            shapeId                = b3CreateSphereShape(bodyId, &shapeDef, &sphere);
            break;
        }

        case PhysicsBodyShape::SHAPE_CAPSULE:
        {
            const auto radius = MAX(extent.x, extent.z);
            const auto offset = MAX(0.0f, extent.z - radius);
            const auto center = b3Vec3 {.x = 0, .y = offset, .z = 0};

            b3Capsule capsule = {.center1 = center, .center2 = -center, .radius = radius};
            shapeId           = b3CreateCapsuleShape(bodyId, &shapeDef, &capsule);
            break;
        }

        default:
        {
            throw std::runtime_error(
                std::format("Unhandled physics shape {}", static_cast<int>(shape))
            );
        }
    }

    return std::make_shared<PhysicsBody>(bodyId, shapeId);
}

void Physics::DestroyBody(std::shared_ptr<IPhysicsBody> body)
{
    const auto b = InternalBody(body);
    b3DestroyShape(b->GetShapeID(), false);
    b3DestroyBody(b->GetBodyID());
}

void Physics::EnableBody(std::shared_ptr<IPhysicsBody> body)
{
    const auto b = InternalBody(body);
    b3Body_Enable(b->GetBodyID());
}

void Physics::DisableBody(std::shared_ptr<IPhysicsBody> body)
{
    const auto b = InternalBody(body);
    b3Body_Disable(b->GetBodyID());
}

Transform Physics::GetTransform(std::shared_ptr<IPhysicsBody> body)
{
    const auto b   = InternalBody(body);
    const auto b3t = b3Body_GetTransform(b->GetBodyID());
    auto       t   = Transform();
    t.SetPosition(B3ToGlm(b3t.p));
    t.SetRotation(B3ToGlm(b3t.q));
    return t;
}

void Physics::SetTransform(std::shared_ptr<IPhysicsBody> body, const Transform& t)
{
    const auto b = InternalBody(body);
    b3Body_SetTransform(b->GetBodyID(), GlmToB3(t.GetPosition()), GlmToB3(t.GetRotation()));
}

size_t Physics::MemoryUsage()
{
    return static_cast<size_t>(b3GetByteCount());
}
}; // namespace yar
