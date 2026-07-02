#pragma once

#include "iapp.h"
#include "input.h"
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

    virtual int Run(std::shared_ptr<IApplication> app) = 0;

    virtual WindowInput GetFrameInput() = 0;
    virtual WindowInput GetTickInput()  = 0;
};

extern std::shared_ptr<IEngine> g_engine;
}; // namespace yar
