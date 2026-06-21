#pragma once

#include "../log.h"

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace yar
{
struct Plane
{
    glm::vec3 normal;
    float     distance;

    float SignedDistance(const glm::vec3& pos) const
    {
        return glm::dot(normal, pos) - distance;
    }
};

struct FrustumPlane
{
    glm::vec4 clip;

    float SignedDistance(const glm::vec3& pos) const
    {
        return glm::dot(clip, glm::vec4(pos, 1.0f));
    }
};

struct Frustum
{
    FrustumPlane near;
    FrustumPlane far;
    FrustumPlane top;
    FrustumPlane bottom;
    FrustumPlane left;
    FrustumPlane right;
};

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;
};

struct Sphere
{
    glm::vec3 center;
    float     radius;
};
}; // namespace yar
