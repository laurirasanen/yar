#pragma once

#include "resource.h"

namespace yar
{
class Shader : public Resource
{
  public:
    Shader(const std::string& id) : Resource(id)
    {
    }

  protected:
    bool DoLoad() override
    {
#error implement
        return true;
    }

    bool DoUnload() override
    {
        return true;
    }
};
}; // namespace yar
