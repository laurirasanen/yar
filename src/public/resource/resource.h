#pragma once

#include <string>

namespace yar
{
class Resource
{

  public:
    explicit Resource(const std::string& id) : m_resourceId(id)
    {
    }

    virtual ~Resource() = default;

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
        m_loaded = DoLoad();
        return m_loaded;
    }

    void Unload()
    {
        DoUnload();
        m_loaded = false;
    }

  protected:
    virtual bool DoLoad()   = 0;
    virtual bool DoUnload() = 0;

  private:
    std::string m_resourceId;
    bool        m_loaded = false;
};
}; // namespace yar
