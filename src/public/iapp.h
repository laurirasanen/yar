#pragma once

namespace yar
{
class IApplication
{
  public:
    IApplication()          = default;
    virtual ~IApplication() = default;

    IApplication(const IApplication&)            = delete;
    IApplication(IApplication&&)                 = delete;
    IApplication& operator=(const IApplication&) = delete;
    IApplication& operator=(IApplication&&)      = delete;

    virtual int  Start() = 0;
    virtual void Frame() = 0;
    virtual void Tick()  = 0;
};
}; // namespace yar
