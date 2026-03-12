
#pragma once

#include <glm/vec3.hpp>

#include "Rotation.h"

namespace yar
{
struct Transform
{
    glm::vec3 position;
    Rotation  rotation;
};
} // namespace yar
