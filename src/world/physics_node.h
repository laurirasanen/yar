#pragma once

#include "../public/inode.h"
#include "../public/iphysics.h"

namespace yar
{
class PhysicsNode : public INode
{
  public:
    PhysicsNode(
        std::string      name,
        PhysicsBodyType  type,
        PhysicsBodyShape shape,
        const glm::vec3& position,
        const glm::quat& rotation,
        const glm::vec3& extent
    ) :
        INode(name)
    {
        m_body = g_physics->CreateBody(type, shape, position, rotation, extent);
        g_physics->EnableBody(m_body);
    }

    ~PhysicsNode()
    {
        g_physics->DestroyBody(m_body);
    }

    void Tick() override
    {
        SetTransform(g_physics->GetTransform(m_body));
        INode::Tick();
    }

    void SetParent(INode*) override
    {
        throw std::runtime_error("Parenting physics nodes is unsupported");
    }

  private:
    std::shared_ptr<IPhysicsBody> m_body;
};
}; // namespace yar
