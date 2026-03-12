#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace yar
{
struct Rotation
{
    // TODO shouldn't need both, get euler from quat
    glm::vec3 euler;
    glm::quat quaternion;
};
} // namespace yar
