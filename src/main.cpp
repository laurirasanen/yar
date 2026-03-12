#include "engine.h"
#include "log.h"

#include <cstring>
#include <exception>

bool HasLaunchArg(const char* name, const char* value, int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (std::strcmp(name, argv[i]) == 0)
        {
            if (value == nullptr)
            {
                return true;
            }
            if (i < argc - 1 && std::strcmp(value, argv[i + 1]) == 0)
            {
                return true;
            }
        }
    }
    return false;
}

int main(/*int argc, char** argv*/)
{
#if NDEBUG
    yar::Log::SetLogLevel(yar::LogLevel::Warning);
#else
    yar::Log::SetLogLevel(yar::LogLevel::Debug);
#endif

    try
    {
        yar::Engine engine;
    }
    catch (std::exception& ex)
    {
        LOG_EXCEPTION("Unhandled exception: {}", ex.what());
        yar::Log::Flush();
        return -1;
    }

    return 0;
}
