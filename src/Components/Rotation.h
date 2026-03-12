#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace drive
{
struct Rotation
{
    // TODO shouldn't need both, get euler from quat
    glm::vec3 euler;
    glm::quat quaternion;
};
} // namespace drive
