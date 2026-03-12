#pragma once

#include "Time.h"

#include <cstdio>
#include <format>
#include <iostream>
#include <mutex>
#include <ostream>

namespace drive
{
enum LogLevel
{
    Debug,
    Info,
    Warning,
    Error,
    Exception,
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
            m_severityStrings[static_cast<int>(level)],
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

  private:
    static inline LogLevel m_logLevel;

    static constexpr const char* m_severityStrings[static_cast<int>(LogLevel::MAX)] = {
        "Debug",
        "Info",
        "Warning",
        "Error",
        "Exception",
    };

    static inline std::mutex m_logMutex;
};

#define _LOG(L, F, ...) \
    drive::Log::Print(__FILE__, __LINE__, __func__, L, F, ##__VA_ARGS__)

#define LOG_DEBUG(F, ...)     _LOG(drive::LogLevel::Debug, F, ##__VA_ARGS__)
#define LOG_INFO(F, ...)      _LOG(drive::LogLevel::Info, F, ##__VA_ARGS__)
#define LOG_WARNING(F, ...)   _LOG(drive::LogLevel::Warning, F, ##__VA_ARGS__)
#define LOG_ERROR(F, ...)     _LOG(drive::LogLevel::Error, F, ##__VA_ARGS__)
#define LOG_EXCEPTION(F, ...) _LOG(drive::LogLevel::Exception, F, ##__VA_ARGS__)

} // namespace drive
