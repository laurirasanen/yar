#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>

namespace yar
{
class ResourceManager;

template<typename T>
class ResourceHandle
{

  public:
    explicit ResourceHandle() : m_initialized(false)
    {
    }

    explicit ResourceHandle(const std::string& id) : m_resourceId(id), m_initialized(true)
    {
    }

    ResourceHandle(const ResourceHandle&);

    ResourceHandle(ResourceHandle&&) = default;

    ResourceHandle& operator=(const ResourceHandle&);

    ResourceHandle& operator=(ResourceHandle&&) = default;

    ~ResourceHandle();

    T* Get() const;

    bool IsValid() const;

    bool IsInitialized() const
    {
        return m_initialized;
    }

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
    std::string m_resourceId  = "";
    bool        m_initialized = false;
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
    const std::string m_resourceId = "";
    bool              m_loaded     = false;
};

class ResourceManager
{
  public:
    ResourceManager()
    {
        m_running      = true;
        m_workerThread = std::thread([this]() { WorkerThread(); });
    }

    ~ResourceManager()
    {
        {
            std::scoped_lock lock {m_queueMutex};
            m_running = false;
        }
        m_condition.notify_one();
        if (m_workerThread.joinable())
        {
            m_workerThread.join();
        }
    }

    template<typename T>
    void AddRef(const ResourceHandle<T>& resource)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        if (!resource.IsValid())
        {
            return;
        }

        auto& typeResources = m_resources[std::type_index(typeid(T))];
        typeResources[resource.GetId()].refCount++;
    }

    template<typename T>
    ResourceHandle<T> Copy(const ResourceHandle<T>& resource)
    {
        AddRef(resource);
        auto& typeResources = m_resources[std::type_index(typeid(T))];
        return typeResources[resource.GetId()];
    }

    template<typename T, typename... Args>
    ResourceHandle<T> Load(const std::string& resourceId, Args&&... args)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        auto& typeResources = m_resources[std::type_index(typeid(T))];

        auto it = typeResources.find(resourceId);
        if (it != typeResources.end())
        {
            it->second.refCount++;
            return ResourceHandle<T>(resourceId);
        }

        auto resource = std::make_unique<T>(resourceId, std::forward<Args>(args)...);
        if (!resource->Load())
        {
            return ResourceHandle<T>();
        }

        typeResources[resourceId] = {.resource = std::move(resource), .refCount = 1};

        return ResourceHandle<T>(resourceId);
    }

    template<typename T>
    void LoadAsync(const std::string& resourceId, std::function<void(ResourceHandle<T>)> callback)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        std::scoped_lock lock {m_queueMutex};
        m_taskQueue.push([this, resourceId, callback]() {
            auto handle = Load<T>(resourceId);
            callback(handle);
        });
        m_condition.notify_one();
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
    void WorkerThread()
    {
        while (m_running)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_condition.wait(lock, [this]() { return !m_taskQueue.empty() || !m_running; });

                if (!m_running && m_taskQueue.empty())
                {
                    return;
                }

                task = std::move(m_taskQueue.front());
                m_taskQueue.pop();
            }
            task();
        }
    }

    bool                              m_running = false;
    std::thread                       m_workerThread;
    std::queue<std::function<void()>> m_taskQueue;
    std::mutex                        m_queueMutex;
    std::condition_variable           m_condition;

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
    if (other.IsInitialized())
    {
        g_resources->AddRef(other);
        m_resourceId  = other.GetId();
        m_initialized = true;
    }
    else
    {
        m_resourceId  = "";
        m_initialized = false;
    }
}

template<typename T>
ResourceHandle<T>& ResourceHandle<T>::operator=(const ResourceHandle<T>& other)
{
    if (this == &other)
    {
        return *this;
    }

    if (other.IsInitialized())
    {
        g_resources->AddRef(other);
        m_resourceId  = other.GetId();
        m_initialized = true;
    }
    else
    {
        m_resourceId  = "";
        m_initialized = false;
    }

    return *this;
}

template<typename T>
ResourceHandle<T>::~ResourceHandle()
{
    if (m_initialized)
    {
        g_resources->Release<T>(m_resourceId);
    }
}

template<typename T>
T* ResourceHandle<T>::Get() const
{
    if (!m_initialized)
    {
#if !NDEBUG
        throw std::runtime_error("Tried to Get() from an uinitialized handle");
#endif

        return nullptr;
    }

    return g_resources->GetResource<T>(m_resourceId);
}

template<typename T>
bool ResourceHandle<T>::IsValid() const
{
    return m_initialized && g_resources->GetResource<T>(m_resourceId) != nullptr;
}
}; // namespace yar
