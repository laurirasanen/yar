#pragma once

#include "../log.h"
#include "component.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace yar
{
class Entity
{
  public:
    explicit Entity(const std::string& name) : m_name(name)
    {
    }

    Entity()          = delete;
    virtual ~Entity() = default;

    Entity(const Entity&)            = default;
    Entity(Entity&&)                 = default;
    Entity& operator=(const Entity&) = default;
    Entity& operator=(Entity&&)      = default;

    void Initialize()
    {
        for (auto& comp : m_components)
        {
            comp->Initialize();
        }
    }

    void Update(float deltaTime)
    {
        if (!m_active)
        {
            return;
        }

        for (auto& comp : m_components)
        {
            comp->Update(deltaTime);
        }
    }

    void EarlyFixedUpdate(float deltaTime)
    {
        if (!m_active)
        {
            return;
        }

        for (auto& comp : m_components)
        {
            comp->EarlyFixedUpdate(deltaTime);
        }
    }

    void FixedUpdate(float deltaTime)
    {
        if (!m_active)
        {
            return;
        }

        for (auto& comp : m_components)
        {
            comp->FixedUpdate(deltaTime);
        }
    }

    void Render()
    {
        if (!m_active)
        {
            return;
        }

        for (auto& comp : m_components)
        {
            comp->Render();
        }
    }

    const std::string& GetName() const
    {
        return m_name;
    }

    bool IsActive() const
    {
        return m_active;
    }

    void SetActive(bool active)
    {
        m_active = active;
    }

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        size_t typeID = Component::GetTypeID<T>();

        auto it = m_componentMap.find(typeID);
        if (it != m_componentMap.end())
        {
            // Returned value may not be what you expect,
            // since previous component could have been
            // constructed with different args.
            LOG_ERROR("Component already exists");
            return static_cast<T*>(it->second);
        }

        auto component         = std::make_unique<T>(this, std::forward<Args>(args)...);
        T*   componentPtr      = component.get();
        m_componentMap[typeID] = componentPtr;
        m_components.push_back(std::move(component));
        return componentPtr;
    }

    template<typename T>
    T* GetComponent()
    {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        size_t typeID = Component::GetTypeID<T>();
        auto   it     = m_componentMap.find(typeID);
        if (it != m_componentMap.end())
        {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }

    template<typename T>
    bool RemoveComponent()
    {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        size_t typeID = Component::GetTypeID<T>();
        auto   it     = m_componentMap.find(typeID);
        if (it != m_componentMap.end())
        {
            Component* componentPtr = it->second;
            m_componentMap.erase(it);

            for (auto compIt = m_components.begin(); compIt != m_components.end(); ++compIt)
            {
                if (compIt->get() == componentPtr)
                {
                    m_components.erase(compIt);
                    return true;
                }
            }
        }
        return false;
    }

  private:
    std::string m_name;

    bool m_active = true;

    std::vector<std::unique_ptr<Component>> m_components;
    std::unordered_map<size_t, Component*>  m_componentMap;
};
}; // namespace yar
