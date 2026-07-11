#pragma once

#include <cstdlib>
#include <cstring>
#include <format>
#include <string>

#if LINUX
extern char** environ;
#endif

namespace yar
{
class Environment
{
  public:
    Environment()                              = delete;
    ~Environment()                             = delete;
    Environment(const Environment&)            = delete;
    Environment(Environment&&)                 = delete;
    Environment& operator=(const Environment&) = delete;
    Environment& operator=(Environment&&)      = delete;

    static std::string GetAll()
    {
        std::string str;

#if LINUX
        for (char** var = environ; *var; var++)
        {
            str += *var;
            str += "\n";
        }
#endif

        return str;
    }

    static bool IsSet(const char* var)
    {
        return std::getenv(var) != nullptr;
    }

    static bool IsTrue(const char* var)
    {
        const char* value = std::getenv(var);

        if (value == nullptr)
        {
            return false;
        }

        const char* truthValues[] = {"true", "True", "TRUE", "1"};

        for (const auto& v : truthValues)
        {
            if (std::strcmp(value, v) == 0)
            {
                return true;
            }
        }

        return false;
    }
};
}; // namespace yar
