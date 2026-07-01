#include <entry.h>
#include <log.h>

using namespace yar;

int main(int argc, char** argv)
{
    if (!YAR_Init(argc, argv))
    {
        return 1;
    }

    g_engine->GetWindow()->SetTitle("example");

    LOG_INFO("Sample app");

    return YAR_Run();
}
