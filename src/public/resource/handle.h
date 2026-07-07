#pragma once

#include <string>

namespace yar
{
class ResourceManager;

template<typename T>
class ResourceHandle
{

  public:
    ResourceHandle() : m_resourceManager(nullptr)
    {
    }

    ResourceHandle(const std::string& id, ResourceManager* manager) :
        m_resourceId(id),
        m_resourceManager(manager)
    {
    }

    T* Get() const;

    bool IsValid() const;

    const std::string& GetId() const
    {
        return m_resourceId;
    }

    T* operator->() const
    {
        return Get();
    }

    T& operator*() const
    {
        return *Get();
    }

    operator bool() const
    {
        return IsValid();
    }

  private:
    std::string      m_resourceId;
    ResourceManager* m_resourceManager;
};
}; // namespace yar
