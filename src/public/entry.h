#include "iapp.h"

#include <memory>

namespace yar
{
extern int YAR_Init(int argc, char** argv);

extern int YAR_Run(std::shared_ptr<IApplication> app);
}; // namespace yar
