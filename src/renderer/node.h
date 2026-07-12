#pragma once

#include "../public/ecs/camera.h"
#include "../public/geometry.h"
#include "../public/material.h"
#include "../public/renderer/data_types.h"
#include "../public/renderer/ibuffer.h"
#include "../public/transform.h"

#include <glm/gtc/constants.hpp>

#include <memory>

namespace yar
{
class Node
{
  public:
    Node() : m_parent(nullptr), m_indexBuffer(nullptr), m_vertices(nullptr)
    {
    }

    ~Node() = default;

    Node(const Node&)            = delete;
    Node(Node&&)                 = delete;
    Node& operator=(const Node&) = delete;
    Node& operator=(Node&&)      = delete;

    const Transform& GetTransform()
    {
        return m_transform;
    }

    void SetTransform(const Transform& transform)
    {
        m_transform = transform;
        UpdateGlobalTransform();
        UpdateAABB();
    }

    const Transform& GetGlobalTransform()
    {
        return m_globalTransform;
    }

    void SetGlobalTransform(const Transform& transform)
    {
        if (m_parent == nullptr)
        {
            SetTransform(transform);
            return;
        }

        SetTransform(transform / m_parent->GetGlobalTransform());
    }

    Node* GetParent()
    {
        return m_parent;
    }

    void SetParent(Node* parent)
    {
        m_parent = parent;
        UpdateGlobalTransform();
        UpdateAABB();
    }

    std::vector<std::shared_ptr<Node>> GetChildren()
    {
        return m_children;
    }

    std::vector<std::shared_ptr<Node>> GetChildrenRecursive()
    {
        std::vector<std::shared_ptr<Node>> children;
        for (const auto& child : m_children)
        {
            children.append_range(child->GetChildrenRecursive());
        }
        children.append_range(m_children);
        return children;
    }

    void AddChild(std::shared_ptr<Node> node)
    {
        node->SetParent(this);
        m_children.push_back(node);
    }

    bool FrustumCull(const Camera* camera)
    {
        return !camera->IsInFrustum(m_aabb);
    }

    void SetAABB(const AABB& aabb)
    {
        m_aabb = aabb;
    }

    AABB GetAABB()
    {
        return m_aabb;
    }

    void UpdateGlobalTransform()
    {
        if (m_parent)
        {
            m_globalTransform = m_parent->GetGlobalTransform() * m_transform;
        }
        else
        {
            m_globalTransform = m_transform;
        }

        for (const auto& child : m_children)
        {
            child->UpdateGlobalTransform();
        }
    }

    void UpdateAABB()
    {
        m_globalAABB = m_aabb.Transform(m_globalTransform);
        for (const auto& child : m_children)
        {
            child->UpdateAABB();
        }
    }

    bool IsRenderable()
    {
        return m_indexBuffer != nullptr && m_vertices != nullptr;
    }

    void SetIndexBuffer(std::shared_ptr<IBuffer> buff)
    {
        m_indexBuffer = buff;
    }

    void SetVertices(std::vector<ShaderVertex>* v)
    {
        m_vertices = v;
    }

    std::shared_ptr<IBuffer> GetIndexBuffer()
    {
        return m_indexBuffer;
    }

    const std::vector<ShaderVertex>* GetVertices() const
    {
        return m_vertices;
    }

    uint32_t GetIndexCount() const
    {
        if (m_indexBuffer == nullptr)
        {
            return 0;
        }
        return m_indexBuffer->GetElementCount();
    }

    uint32_t GetVertexCount() const
    {
        if (m_vertices == nullptr)
        {
            return 0;
        }
        return static_cast<uint32_t>(m_vertices->size());
    }

    void SetMaterial(const Material& mat)
    {
        m_material = mat;
    }

    Material GetMaterial()
    {
        return m_material;
    }

  private:
    Transform                          m_transform;
    Transform                          m_globalTransform;
    AABB                               m_aabb;
    AABB                               m_globalAABB;
    Node*                              m_parent;
    std::vector<std::shared_ptr<Node>> m_children;
    Material                           m_material;
    std::shared_ptr<IBuffer>           m_indexBuffer;
    std::vector<ShaderVertex>*         m_vertices;
};
}; // namespace yar
