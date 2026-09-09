#pragma once

#include "../engine/iphysics.h"
#include "collider.h"
#include "component.h"
#include "entity.h"
#include "transform.h"

namespace yar
{
class RigidBodyComponent : public Component
{
  public:
    RigidBodyComponent(Entity* owner, PhysicsBodyType type) : Component(owner), m_type(type)
    {
        auto transform  = m_owner->GetComponent<TransformComponent>()->GetTransform();
        m_prevTransform = *transform;
        m_nextTransform = m_prevTransform;
        m_lerp          = 0;

        m_body = g_physics->CreateBody(m_type, transform->GetPosition(), transform->GetRotation());
    }

    void OnInitialize() override
    {
        g_physics->EnableBody(m_body);
        if (m_wantLinearVelocity)
        {
            SetLinearVelocity(m_spawnLinearVelocity);
            m_wantLinearVelocity = false;
        }
        if (m_wantAngularVelocity)
        {
            SetAngularVelocity(m_spawnAngularVelocity);
            m_wantAngularVelocity = false;
        }
    }

    void Update(float deltaTime) override
    {
        if (m_type != PhysicsBodyType::BODY_DYNAMIC)
        {
            return;
        }
        m_lerp         = static_cast<float>(Time::TimeSinceEngineTick() / Time::DeltaTick);
        auto transform = m_owner->GetComponent<TransformComponent>()->GetTransform();
        *transform     = Transform::Lerp(m_prevTransform, m_nextTransform, m_lerp);
    }

    void FixedUpdate(float deltaTime) override
    {
        if (m_type != PhysicsBodyType::BODY_DYNAMIC)
        {
            return;
        }

        auto transform  = m_owner->GetComponent<TransformComponent>()->GetTransform();
        m_prevTransform = *transform;
        m_nextTransform = g_physics->GetTransform(m_body);
        // physics system has no scale
        m_nextTransform.Scale(transform->GetScale());
        m_lerp = 0;
    }

    void AddCollider(
        PhysicsShapeType type,
        const glm::vec3& position,
        const glm::quat& rotation,
        const glm::vec3& size
    )
    {
        g_physics->AddShape(m_body, type, position, rotation, size);
    }

    glm::vec3 GetLinearVelocity()
    {
        return g_physics->GetLinearVelocity(m_body);
    }

    void SetLinearVelocity(const glm::vec3 vel)
    {
        if (m_state != State::Initializing && m_state != State::Active)
        {
            m_wantLinearVelocity  = true;
            m_spawnLinearVelocity = vel;
            return;
        }
        g_physics->SetLinearVelocity(m_body, vel);
    }

    glm::vec3 GetAngularVelocity()
    {
        return g_physics->GetAngularVelocity(m_body);
    }

    void SetAngularVelocity(const glm::vec3 vel)
    {
        if (m_state != State::Initializing && m_state != State::Active)
        {
            m_wantAngularVelocity  = true;
            m_spawnAngularVelocity = vel;
            return;
        }
        g_physics->SetAngularVelocity(m_body, vel);
    }

  private:
    PhysicsBodyType               m_type;
    std::shared_ptr<IPhysicsBody> m_body;

    Transform m_prevTransform;
    Transform m_nextTransform;

    float m_lerp;

    bool      m_wantLinearVelocity;
    bool      m_wantAngularVelocity;
    glm::vec3 m_spawnLinearVelocity;
    glm::vec3 m_spawnAngularVelocity;
};
}; // namespace yar
