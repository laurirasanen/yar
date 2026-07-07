#pragma once

#include "handle.h"
#include "resource.h"

#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace yar
{
class ResourceManager
{
  public:
    template<typename T, typename... Args>
    ResourceHandle<T> Load(const std::string& resourceId, Args&&... args)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        auto& typeResources = m_resources[std::type_index(typeid(T))];

        auto it = typeResources.find(resourceId);
        if (it != typeResources.end())
        {
            it->second.refCount++;
            return ResourceHandle<T>(resourceId, this);
        }

        auto resource = std::make_unique<T>(resourceId, std::forward<Args>(args)...);
        if (!resource->Load())
        {
            return ResourceHandle<T>();
        }

        typeResources[resourceId] = {.resource = std::move(resource), .refCount = 1};

        return ResourceHandle<T>(resourceId, this);
    }

    template<typename T>
    T* GetResource(const std::string& resourceId)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        auto& typeResources = m_resources[std::type_index(typeid(T))];
        auto  it            = typeResources.find(resourceId);

        if (it != typeResources.end())
        {
            return static_cast<T*>(it->second.resource.get());
        }

        return nullptr;
    }

    template<typename T>
    void Release(const std::string& resourceId)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        auto& typeResources = m_resources[std::type_index(typeid(T))];
        auto  it            = typeResources.find(resourceId);
        if (it != typeResources.end())
        {
            it->second.refCount--;

            if (it->second.refCount <= 0)
            {
                it->second.resource->Unload();
                typeResources.erase(it);
            }
        }
    }

    void UnloadAll()
    {
        for (auto& [type, refCounts] : m_resources)
        {
            for (auto& [id, data] : refCounts)
            {
                data.resource->Unload();
            }
            refCounts.clear();
        }
        m_resources.clear();
    }

  private:
    struct RefCountedResource
    {
        std::unique_ptr<Resource> resource;
        uint32_t                  refCount;
    };

    std::unordered_map<std::type_index, std::unordered_map<std::string, RefCountedResource>>
        m_resources;
};

template<typename T>
T* ResourceHandle<T>::Get() const
{
    if (!m_resourceManager)
    {
        return nullptr;
    }
    return m_resourceManager->GetResource<T>(m_resourceId);
}

template<typename T>
bool ResourceHandle<T>::IsValid() const
{
    if (!m_resourceManager)
    {
        return false;
    }
    return m_resourceManager->GetResource<T>(m_resourceId) != nullptr;
}
}; // namespace yar
