#pragma once

#include <chrono>

namespace yar
{
#define ARRAY_SIZE(A) (sizeof(A) / sizeof(*A))

#define RAND_DOUBLE(D)    (D * static_cast<double>(rand()) / RAND_MAX)
#define RAND_FLOAT(F)     (static_cast<float>(RAND(static_cast<double>(F))))
#define RAND_INT(I)       (rand() % I)
#define RAND_ELEMENT(ARR) (ARR[RAND_INT(ARRAY_SIZE(ARR))])

#define RAND_STR(LEN, STR)                                                   \
    {                                                                        \
        const char characters[] = {                                          \
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', \
            'o', 'n', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'  \
        };                                                                   \
        STR.resize(LEN);                                                     \
        for (int i = 0; i < LEN; i++)                                        \
        {                                                                    \
            STR[i] = RAND_ELEMENT(characters);                               \
        }                                                                    \
    }

using clock_duration = std::chrono::duration<
    double,
    std::ratio<
        std::chrono::high_resolution_clock::period::num,
        std::chrono::high_resolution_clock::period::den>>;

class Time
{
  public:
    static double Now()
    {
        const auto now = std::chrono::high_resolution_clock::now();
        return static_cast<double>(now.time_since_epoch().count())
               * std::chrono::high_resolution_clock::period::num
               / std::chrono::high_resolution_clock::period::den;
    }

    static double Uptime()
    {
        return Now() - startTime;
    }

    constexpr static clock_duration Duration(const double seconds)
    {
        return clock_duration {
            seconds * std::chrono::high_resolution_clock::period::den
            / std::chrono::high_resolution_clock::period::num
        };
    }

    static void UpdateFrameDelta()
    {
        const auto now = Now();
        DeltaFrame     = now - prevFrame;
        prevFrame      = now;
    }

    static void UpdateTickDelta()
    {
        const auto now = Now();
        DeltaTick      = now - prevTick;
        prevTick       = now;
    }

    static double TimeSinceEngineFrame()
    {
        return Now() - prevFrame;
    }

    static double TimeSinceEngineTick()
    {
        return Now() - prevTick;
    }

    static bool TimeForEngineFrame()
    {
        return TimeSinceEngineFrame() >= FrameInterval;
    }

    static bool TimeForEngineTick()
    {
        return TimeSinceEngineTick() >= TickInterval;
    }

    static void SetStart()
    {
        startTime = Now();
    }

    static void SetFrameRate(unsigned int fps)
    {
        FrameRate     = fps;
        FrameInterval = 1.0 / fps;
    }

    static void StartRender()
    {
        renderStart = Now();
    }

    static void StopRender()
    {
        DeltaRender = Now() - renderStart;
    }

    static inline double DeltaFrame;
    static inline double DeltaTick;
    static inline double DeltaRender;

    static inline unsigned int FrameRate     = 60;
    static inline double       FrameInterval = 1.0 / FrameRate;

    static constexpr const unsigned int TickRate     = 60;
    static constexpr const double       TickInterval = 1.0 / TickRate;

  private:
    static inline double prevFrame;
    static inline double prevTick;
    static inline double renderStart;
    static inline double startTime;
};
}; // namespace yar
