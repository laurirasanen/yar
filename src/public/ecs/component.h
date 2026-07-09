#pragma once

#include <cstddef>

namespace yar
{
class Entity;

class ComponentTypeIDSystem
{
  private:
    static size_t nextTypeID;

  public:
    template<typename T>
    static size_t GetTypeID()
    {
        static size_t typeID = nextTypeID++;
        return typeID;
    }
};

size_t ComponentTypeIDSystem::nextTypeID = 0;

class Component
{
  public:
    enum class State
    {
        Uninitialized,
        Initializing,
        Active,
        Destroying,
        Destroyed
    };

    Component() = default;

    virtual ~Component()
    {
        if (m_state != State::Destroyed)
        {
            m_state = State::Destroying;
            OnDestroy();
            m_state = State::Destroyed;
        }
    }

    Component(const Component&)            = default;
    Component(Component&&)                 = default;
    Component& operator=(const Component&) = default;
    Component& operator=(Component&&)      = default;

    virtual void Initialize()
    {
        if (m_state == State::Uninitialized)
        {
            m_state = State::Initializing;
            OnInitialize();
            m_state = State::Active;
        }
    }

    void Destroy()
    {
        if (m_state == State::Active)
        {
            m_state = State::Destroying;
            OnDestroy();
            m_state = State::Destroyed;
        }
    }

    bool IsActive() const
    {
        return m_state == State::Active;
    }

    void SetOwner(Entity* entity)
    {
        m_owner = entity;
    }

    Entity* GetOwner() const
    {
        return m_owner;
    }

    template<typename T>
    static size_t GetTypeID()
    {
        return ComponentTypeIDSystem::GetTypeID<T>();
    }

  protected:
    virtual void OnInitialize()
    {
    }

    virtual void OnDestroy()
    {
    }

    virtual void Update(float deltaTime)
    {
    }

    virtual void EarlyFixedUpdate(float deltaTime)
    {
    }

    virtual void FixedUpdate(float deltaTime)
    {
    }

    virtual void Render()
    {
    }

    Entity* m_owner;
    State   m_state;

    friend class Entity;
};
}; // namespace yar
