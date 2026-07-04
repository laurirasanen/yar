#pragma once

#include "camera.h"
#include "geometry.h"
#include "imaterial.h"
#include "transform.h"

#include <glm/gtc/constants.hpp>
#include <memory>

namespace yar
{
class INode
{
  public:
    INode(std::string name) :
        m_name(name),
        m_transform({}),
        m_oldGlobalTransform({}),
        m_globalTransform({}),
        m_aabb({}),
        m_parent(nullptr),
        m_children({})
    {
    }

    virtual ~INode() = default;

    INode(const INode&)            = delete;
    INode(INode&&)                 = delete;
    INode& operator=(const INode&) = delete;
    INode& operator=(INode&&)      = delete;

    std::string GetName() const
    {
        return m_name;
    }

    const Transform& GetTransform()
    {
        return m_transform;
    }

    void SetTransform(const Transform& transform, bool teleport = false)
    {
        m_transform = transform;
        UpdateGlobalTransform(teleport);
        UpdateAABB();
    }

    const Transform& GetGlobalTransform()
    {
        return m_globalTransform;
    }

    Transform GetRenderTransform(float lerp)
    {
        if (m_interpolate)
        {
            return Transform::Lerp(m_oldGlobalTransform, m_globalTransform, lerp);
        }

        return m_globalTransform;
    }

    void SetGlobalTransform(const Transform& transform, bool teleport = false)
    {
        if (m_parent == nullptr)
        {
            SetTransform(transform, teleport);
            return;
        }

        SetTransform(transform / m_parent->GetGlobalTransform(), teleport);
    }

    INode* GetParent()
    {
        return m_parent;
    }

    virtual void SetParent(INode* parent)
    {
        m_parent = parent;
        UpdateGlobalTransform(true);
        UpdateAABB();
    }

    std::vector<std::shared_ptr<INode>> GetChildren()
    {
        return m_children;
    }

    std::vector<std::shared_ptr<INode>> GetChildrenRecursive()
    {
        std::vector<std::shared_ptr<INode>> children;
        for (const auto& child : m_children)
        {
            children.append_range(child->GetChildrenRecursive());
        }
        children.append_range(m_children);
        return children;
    }

    void AddChild(std::shared_ptr<INode> node)
    {
        node->SetParent(this);
        m_children.push_back(node);
    }

    virtual AABB GetAABB()
    {
        return m_aabb;
    }

    virtual bool Renderable() const
    {
        return false;
    }

    bool FrustumCull(const std::shared_ptr<Camera>& camera)
    {
        return !camera->IsInFrustum(m_aabb);
    }

    virtual uint32_t GetVertexCount() const
    {
        return 0;
    }

    virtual uint32_t GetIndexCount() const
    {
        return 0;
    }

    virtual std::shared_ptr<IMaterial> GetMaterial()
    {
        return nullptr;
    }

    virtual void EarlyTick()
    {
        m_interpolate = false;
    }

    virtual void Tick()
    {
        for (const auto& child : m_children)
        {
            child->Tick();
        }
    };

    virtual void BindPipeline() {};

    virtual void Render() {};

    void UpdateGlobalTransform(bool teleport = false)
    {
        if (!teleport)
        {
            m_oldGlobalTransform = m_globalTransform;
            m_interpolate        = true;
        }

        if (m_parent)
        {
            m_globalTransform = m_parent->GetGlobalTransform() * m_transform;
        }
        else
        {
            m_globalTransform = m_transform;
        }

        if (teleport)
        {
            m_oldGlobalTransform = m_globalTransform;
        }

        for (const auto& child : m_children)
        {
            child->UpdateGlobalTransform(teleport);
        }
    }

    virtual void UpdateAABB()
    {
        for (const auto& child : m_children)
        {
            child->UpdateAABB();
        }
    }

  protected:
    std::string                         m_name;
    Transform                           m_transform;
    Transform                           m_oldGlobalTransform;
    Transform                           m_globalTransform;
    AABB                                m_aabb;
    INode*                              m_parent;
    std::vector<std::shared_ptr<INode>> m_children;
    bool                                m_interpolate;
};
}; // namespace yar
