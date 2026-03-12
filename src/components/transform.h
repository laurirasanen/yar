
#pragma once

#include <glm/vec3.hpp>

#include "rotation.h"

namespace yar
{
struct Transform
{
    glm::vec3 position;
    Rotation  rotation;
};
} // namespace yar
