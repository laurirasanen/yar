#include "Engine.h"
#include "Log.h"

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

int main(int argc, char** argv)
{
#if NDEBUG
    drive::Log::SetLogLevel(drive::LogLevel::Warning);
#else
    drive::Log::SetLogLevel(drive::LogLevel::Debug);
#endif

    try
    {
        auto rendererType = drive::RendererType::VULKAN;
        if (HasLaunchArg("-renderer", "empty", argc, argv))
        {
            rendererType = drive::RendererType::EMPTY;
        }
        drive::Engine engine(rendererType);
    }
    catch (std::exception& ex)
    {
        LOG_EXCEPTION("Unhandled exception: {}", ex.what());
        drive::Log::Flush();
        return -1;
    }

    return 0;
}
