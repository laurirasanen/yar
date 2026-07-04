#include "../public/log.h"
#include "../public/time_util.h"

#include <cmath>

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
            "Engine: Tick ran slow at {}, clamp delta to {}",
            Pretty(DeltaTick),
            Pretty(SlowTickThreshold)
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
    prevTick  = startTime - TickInterval;
    prevFrame = startTime - FrameInterval;
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

std::string Time::Pretty(double seconds)
{
    if (seconds < 1e-6)
    {
        return std::format("{:.1f}ns", seconds * 1e9);
    }
    if (seconds < 1e-3)
    {
        return std::format("{:.1f}us", seconds * 1e6);
    }
    if (seconds < 1)
    {
        return std::format("{:.1f}ms", seconds * 1e3);
    }
    if (seconds < 60)
    {
        return std::format("{:.1f}s", seconds);
    }
    if (seconds < 3600)
    {
        const uint32_t m = static_cast<uint32_t>(floor(seconds / 60));
        const double   s = seconds - m * 60.0;
        return std::format("{}m {:.1f}s", m, s);
    }

    return std::format("{:.1f}s", seconds);
}
}; // namespace yar
