#pragma once

#include <chrono>

namespace yar
{
using clock_duration = std::chrono::duration<
    double,
    std::ratio<
        std::chrono::high_resolution_clock::period::num,
        std::chrono::high_resolution_clock::period::den>>;

class Time
{
  public:
    constexpr static clock_duration Duration(const double seconds)
    {
        return clock_duration {
            seconds * std::chrono::high_resolution_clock::period::den
            / std::chrono::high_resolution_clock::period::num
        };
    }

    static double Now();

    static double Uptime();

    static void UpdateFrameDelta();

    static void UpdateTickDelta();

    static double TimeSinceEngineFrame();

    static double TimeSinceEngineTick();

    static bool TimeForEngineFrame();

    static bool TimeForEngineTick();

    static void SetStart();

    static void SetFrameRate(unsigned int fps);

    static void StartRender();

    static void StopRender();

    static inline double DeltaFrame;
    static inline double DeltaTick;
    static inline double DeltaRender;

    static inline unsigned int FrameRate     = 60;
    static inline double       FrameInterval = 1.0 / FrameRate;

    static constexpr const unsigned int TickRate          = 60;
    static constexpr const double       TickInterval      = 1.0 / TickRate;
    static constexpr const double       SlowTickThreshold = 2.0 * TickInterval;

  private:
    static inline double prevFrame;
    static inline double prevTick;
    static inline double renderStart;
    static inline double startTime;
};
}; // namespace yar
