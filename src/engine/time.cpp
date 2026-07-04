#include "../public/log.h"
#include "../public/time_util.h"

namespace yar
{
double Time::Now()
{
    const auto now = std::chrono::high_resolution_clock::now();
    return static_cast<double>(now.time_since_epoch().count())
           * std::chrono::high_resolution_clock::period::num
           / std::chrono::high_resolution_clock::period::den;
}

double Time::Uptime()
{
    return Now() - startTime;
}

void Time::UpdateFrameDelta()
{
    const auto now = Now();
    DeltaFrame     = now - prevFrame;
    prevFrame      = now;
}

void Time::UpdateTickDelta()
{
    const auto now = Now();
    DeltaTick      = now - prevTick;
    prevTick       = now;

    if (DeltaTick > SlowTickThreshold)
    {
        LOG_WARN(
            "Engine: Tick ran slow {:.2}ms, clamp delta {:.2}ms",
            DeltaTick * 1000,
            SlowTickThreshold * 1000
        );
        DeltaTick = SlowTickThreshold;
    }
}

double Time::TimeSinceEngineFrame()
{
    return Now() - prevFrame;
}

double Time::TimeSinceEngineTick()
{
    return Now() - prevTick;
}

double Time::TickFraction()
{
    return TimeSinceEngineTick() / TickInterval;
}

bool Time::TimeForEngineFrame()
{
    return TimeSinceEngineFrame() >= FrameInterval;
}

bool Time::TimeForEngineTick()
{
    return TimeSinceEngineTick() >= TickInterval;
}

void Time::SetStart()
{
    startTime = Now();
}

void Time::SetFrameRate(unsigned int fps)
{
    FrameRate     = fps;
    FrameInterval = 1.0 / fps;
}

void Time::StartRender()
{
    renderStart = Now();
}

void Time::StopRender()
{
    DeltaRender = Now() - renderStart;
}
}; // namespace yar
