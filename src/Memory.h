#pragma once

#if POSIX
#include <sys/resource.h>
#include <sys/time.h>
#elif _WIN
#define WIN32_LEAN_AND_MEAN
#include <Psapi.h>
#include <windows.h>
#endif

namespace drive
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

#elif _WIN
        PROCESS_MEMORY_COUNTERS counters = {};
        if (K32GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
        {
            usage = static_cast<unsigned long long int>(counters.WorkingSetSize);
        }
#endif

        return usage;
    }
};
}; // namespace drive
