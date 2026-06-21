#pragma once

#include <format>
#include <string>

#if POSIX
#include <sys/resource.h>
#include <sys/time.h>
#elif _WIN
#define WIN32_LEAN_AND_MEAN
#include <Psapi.h>
#include <windows.h>
#endif

#define KILO 1024
#define MEGA (1024 * 1024)
#define GIGA (1024 * 1024 * 1024)

#define TO_KB(B) (static_cast<double>(B) / KILO)
#define TO_MB(B) (static_cast<double>(B) / MEGA)
#define TO_GB(B) (static_cast<double>(B) / GIGA)

namespace yar
{
class Memory
{
  public:
    // Get resident memory usage in KB.
    static unsigned long long int GetUsage()
    {
        unsigned long long int usage = 0;

#if POSIX
        rusage ru = {};
        if (getrusage(RUSAGE_SELF, &ru) == 0)
        {
            usage = static_cast<unsigned long long int>(ru.ru_maxrss);
        }

#elif WIN64
        PROCESS_MEMORY_COUNTERS counters = {};
        if (K32GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
        {
            usage = static_cast<unsigned long long int>(counters.WorkingSetSize);
        }
#endif

        return usage;
    }

    static std::string Pretty(size_t bytes)
    {
        if (bytes >= GIGA)
        {
            return std::format("{:.2f} GB", TO_GB(bytes));
        }
        else if (bytes >= MEGA)
        {
            return std::format("{:.2f} MB", TO_MB(bytes));
        }
        else if (bytes >= KILO)
        {
            return std::format("{:.2f} KB", TO_KB(bytes));
        }

        return std::format("{} B", bytes);
    }

    static std::string GetPrettyUsage()
    {
        return Pretty(GetUsage() * 1024);
    }
};
}; // namespace yar
