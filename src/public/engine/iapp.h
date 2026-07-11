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

    virtual int  Start()                      = 0;
    virtual void Update(float deltaTime)      = 0;
    virtual void FixedUpdate(float deltaTime) = 0;
};
}; // namespace yar
