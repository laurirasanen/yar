#pragma once

#include "time.h"

#include <cstdio>
#include <format>
#include <iostream>
#include <mutex>
#include <ostream>

namespace yar
{
enum LogLevel
{
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    MAX,
};

class Log
{
  public:
    static void SetLogLevel(const LogLevel level)
    {
        m_logLevel = level;
    }

    template<typename... Args>
    static void Print(
        const char*    file,
        const int      line,
        const char*    func,
        const LogLevel level,
        const char*    fmt,
        Args&&... args
    )
    {
        if (level < m_logLevel)
        {
            return;
        }

        const std::scoped_lock lock {m_logMutex};

        auto vargs = std::vformat(fmt, std::make_format_args(args...));

        auto message = std::format(
            "[{:.6f}]"      // Time
            "[{}]"          // Severity
            "[{}:{}@{}()] " // Location
            "{}",           // Message
            Time::Now(),
            SeverityStrings[static_cast<int>(level)],
            file,
            line,
            func,
            vargs
        );

        std::cout << message << '\n';

#if !NDEBUG
        Flush();
#endif
    }

    static void Flush()
    {
        std::flush(std::cout);
    }

    static constexpr const char* SeverityStrings[static_cast<int>(LogLevel::MAX)] = {
        "debug",
        "info",
        "warn",
        "error",
        "fatal",
    };

  private:
    static inline LogLevel m_logLevel;

    static inline std::mutex m_logMutex;
};

#define _LOG(L, F, ...) yar::Log::Print(__FILE__, __LINE__, __func__, L, F, ##__VA_ARGS__)

#define LOG_DEBUG(F, ...) _LOG(yar::LogLevel::Debug, F, ##__VA_ARGS__)
#define LOG_INFO(F, ...)  _LOG(yar::LogLevel::Info, F, ##__VA_ARGS__)
#define LOG_WARN(F, ...)  _LOG(yar::LogLevel::Warn, F, ##__VA_ARGS__)
#define LOG_ERROR(F, ...) _LOG(yar::LogLevel::Error, F, ##__VA_ARGS__)
#define LOG_FATAL(F, ...) _LOG(yar::LogLevel::Fatal, F, ##__VA_ARGS__)

} // namespace yar
