#include "../public/entry.h"
#include "../public/log.h"
#include "engine.h"

#include <cstring>
#include <exception>

namespace yar
{
std::shared_ptr<IEngine> g_engine;

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

char* GetLaunchArg(const char* name, int argc, char** argv)
{
    for (int i = 0; i < argc - 1; i++)
    {
        if (std::strcmp(name, argv[i]) == 0)
        {
            return argv[i + 1];
        }
    }
    return nullptr;
}

int YAR_Init(int argc, char** argv)
{
#if NDEBUG
    auto logLevel = yar::LogLevel::Warn;
#else
    auto logLevel = yar::LogLevel::Debug;
#endif

    auto logLevelArg = GetLaunchArg("-loglevel", argc, argv);
    if (logLevelArg)
    {
        for (int i = 0; i < yar::LogLevel::MAX; i++)
        {
            if (std::strcmp(logLevelArg, yar::Log::SeverityStrings[i]) == 0)
            {
                logLevel = static_cast<yar::LogLevel>(i);
                break;
            }
        }
    }

    yar::Log::SetLogLevel(logLevel);

    g_engine = std::make_shared<yar::Engine>();

    return 1;
}

int YAR_Run(std::shared_ptr<IApplication> app)
{
    auto code = g_engine->Run(app);
    g_engine.reset();
    return code;
}
} // namespace yar
