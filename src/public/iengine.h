#pragma once

#include "iwindow.h"

#include <memory>

namespace yar
{
class IEngine
{
  public:
    IEngine()          = default;
    virtual ~IEngine() = default;

    IEngine(const IEngine&)            = delete;
    IEngine(IEngine&&)                 = delete;
    IEngine& operator=(const IEngine&) = delete;
    IEngine& operator=(IEngine&&)      = delete;

    virtual int Run() = 0;

    virtual std::shared_ptr<IWindow> GetWindow() = 0;
};
}; // namespace yar
