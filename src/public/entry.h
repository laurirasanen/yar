#include "iengine.h"

#include <memory>

namespace yar
{
extern std::shared_ptr<IEngine> g_engine;

extern int YAR_Init(int argc, char** argv);

extern int YAR_Run();
}; // namespace yar
