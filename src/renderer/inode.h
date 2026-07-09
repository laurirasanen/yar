#pragma once

#include "camera.h"
#include "geometry.h"
#include "imaterial.h"
#include "irenderer.h"
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

    const TransformComponent& GetTransform()
    {
        return m_transform;
    }

    void SetTransform(const TransformComponent& transform, bool teleport = false)
    {
        m_transform = transform;
        UpdateGlobalTransform(teleport);
        UpdateAABB();
    }

    const TransformComponent& GetGlobalTransform()
    {
        return m_globalTransform;
    }

    TransformComponent GetInterpolatedTransform(float lerp)
    {
        if (m_interpolate)
        {
            return TransformComponent::Lerp(m_oldGlobalTransform, m_globalTransform, lerp);
        }

        return m_globalTransform;
    }

    void SetGlobalTransform(const TransformComponent& transform, bool teleport = false)
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

    bool FrustumCull(const std::shared_ptr<Camera>& camera)
    {
        return !camera->IsInFrustum(m_aabb);
    }

    virtual AABB GetAABB()
    {
        return m_aabb;
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
    TransformComponent                           m_transform;
    TransformComponent                           m_oldGlobalTransform;
    TransformComponent                           m_globalTransform;
    AABB                                m_aabb;
    INode*                              m_parent;
    std::vector<std::shared_ptr<INode>> m_children;
    bool                                m_interpolate;
};

class IRenderNode : public INode
{
  public:
    IRenderNode(std::string name, std::shared_ptr<IMaterial> material, RenderPipeline pipe) :
        INode(name),
        m_material(material),
        m_pipeline(pipe)
    {
    }

    virtual ~IRenderNode() = default;

    IRenderNode(const IRenderNode&)            = delete;
    IRenderNode(IRenderNode&&)                 = delete;
    IRenderNode& operator=(const IRenderNode&) = delete;
    IRenderNode& operator=(IRenderNode&&)      = delete;

    virtual uint32_t GetVertexCount() const
    {
        return 0;
    }

    virtual uint32_t GetIndexCount() const
    {
        return 0;
    }

    std::shared_ptr<IMaterial> GetMaterial()
    {
        return m_material;
    }

    RenderPipeline GetPipeline()
    {
        return m_pipeline;
    }

    virtual void Render() const = 0;

  private:
    std::shared_ptr<IMaterial> m_material;
    RenderPipeline             m_pipeline;
};
}; // namespace yar
