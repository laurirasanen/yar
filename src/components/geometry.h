#pragma once

#include "../log.h"

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace yar
{
struct Plane
{
    glm::vec3 normal;
    float     distance;

    float SignedDistance(const glm::vec3& pos)
    {
        return glm::dot(normal, pos) - distance;
    }
};

struct Frustum
{
    Plane near;
    Plane far;
    Plane top;
    Plane bottom;
    Plane left;
    Plane right;
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
