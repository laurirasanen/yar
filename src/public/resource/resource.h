#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

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

    ResourceHandle(const ResourceHandle&);
    ResourceHandle(ResourceHandle&&);
    ResourceHandle& operator=(const ResourceHandle&);
    ResourceHandle& operator=(ResourceHandle&&);

    ~ResourceHandle();

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

class Resource
{

  public:
    explicit Resource(const std::string& id) : m_resourceId(id)
    {
    }

    Resource()                           = delete;
    Resource(const Resource&)            = delete;
    Resource(Resource&&)                 = delete;
    Resource& operator=(const Resource&) = delete;
    Resource& operator=(Resource&&)      = delete;

    virtual ~Resource()
    {
        Unload();
    }

    const std::string& GetId() const
    {
        return m_resourceId;
    }

    bool IsLoaded() const
    {
        return m_loaded;
    }

    bool Load()
    {
        if (m_loaded)
        {
            return false;
        }
        m_loaded = DoLoad();
        return m_loaded;
    }

    void Unload()
    {
        if (!m_loaded)
        {
            return;
        }
        DoUnload();
        m_loaded = false;
    }

  protected:
    virtual bool DoLoad()   = 0;
    virtual bool DoUnload() = 0;

  private:
    const std::string m_resourceId;
    bool              m_loaded = false;
};

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

extern std::shared_ptr<ResourceManager> g_resources;

template<typename T>
ResourceHandle<T>::ResourceHandle(const ResourceHandle<T>& other)
{
    return g_resources->Load<T>(other.GetId());
}

template<typename T>
ResourceHandle<T>::ResourceHandle(ResourceHandle<T>&& other) = default;

template<typename T>
ResourceHandle<T>& ResourceHandle<T>::operator=(const ResourceHandle<T>& other)
{
    return g_resources->Load<T>(other.GetId());
}

template<typename T>
ResourceHandle<T>& ResourceHandle<T>::operator=(ResourceHandle<T>&& other) = default;

template<typename T>
ResourceHandle<T>::~ResourceHandle()
{
    if (!m_resourceManager)
    {
        return;
    }
    m_resourceManager->Release<T>(m_resourceId);
}

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
