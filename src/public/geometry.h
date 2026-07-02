#pragma once

#include "transform.h"

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
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
    glm::vec3 center;

    AABB Transform(const Transform& t) const
    {
        const glm::vec3 corners[8] = {
            t.ToGlobalSpace(glm::vec4(min.x, min.y, min.z, 1.0f)),
            t.ToGlobalSpace(glm::vec4(max.x, min.y, min.z, 1.0f)),
            t.ToGlobalSpace(glm::vec4(min.x, max.y, min.z, 1.0f)),
            t.ToGlobalSpace(glm::vec4(max.x, max.y, min.z, 1.0f)),
            t.ToGlobalSpace(glm::vec4(min.x, min.y, max.z, 1.0f)),
            t.ToGlobalSpace(glm::vec4(max.x, min.y, max.z, 1.0f)),
            t.ToGlobalSpace(glm::vec4(min.x, max.y, max.z, 1.0f)),
            t.ToGlobalSpace(glm::vec4(max.x, max.y, max.z, 1.0f)),
        };

        AABB aabb;

        aabb.min = corners[0];
        aabb.max = corners[0];

        for (uint8_t i = 1; i < 8; i++)
        {
            aabb.min = glm::min(aabb.min, corners[i]);
            aabb.max = glm::max(aabb.max, corners[i]);
        }

        aabb.center = aabb.min + 0.5f * (aabb.max - aabb.min);

        return aabb;
    }
};

struct Sphere
{
    glm::vec3 center;
    float     radius;
};

struct Rect
{
    glm::ivec2 size;
    glm::ivec2 offset;
};
}; // namespace yar
