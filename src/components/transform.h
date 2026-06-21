#pragma once

#include <cmath>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>

namespace yar
{
struct Transform
{
    glm::mat4 matrix = glm::identity<glm::mat4>();

    void SetPosition(glm::vec3 pos)
    {
        matrix[3][0] = pos.x;
        matrix[3][1] = pos.y;
        matrix[3][2] = pos.z;
    }

    glm::vec3 GetPosition() const
    {
        return glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
    }

    void Scale(const glm::vec3 scale)
    {
        matrix[0] *= scale.x;
        matrix[1] *= scale.y;
        matrix[2] *= scale.z;
    }

    void SetScale(const glm::vec3 scale)
    {
        Scale(scale / GetScale());
    }

    glm::vec3 GetScale() const
    {
        return glm::vec3(
            glm::length(glm::vec3(matrix[0])),
            glm::length(glm::vec3(matrix[1])),
            glm::length(glm::vec3(matrix[2]))
        );
    }

    glm::vec3 GetEulerRotation() const
    {
        const auto quat  = GetRotation();
        const auto euler = glm::eulerAngles(quat);
        return glm::degrees(euler);
    }

    void SetEulerRotation(const glm::vec3 angles)
    {
        SetRotation(
            glm::angleAxis(glm::radians(angles.z), glm::vec3 {0.0f, 0.0f, 1.0f})
            * glm::angleAxis(glm::radians(angles.x), glm::vec3 {1.0f, 0.0f, 0.0f})
        );
    }

    glm::quat GetRotation() const
    {
        return glm::quat_cast(matrix);
    }

    void SetRotation(glm::quat rotation)
    {
        matrix /= glm::mat4_cast(GetRotation());
        matrix *= glm::mat4_cast(rotation);
    }

    glm::vec3 Forward() const
    {
        return GetRotation() * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::vec3 Right() const
    {
        return GetRotation() * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 Up() const
    {
        return GetRotation() * glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::vec3 ToGlobalSpace(const glm::vec3 local) const
    {
        return glm::vec3(matrix * glm::vec4(local, 1.0));
    }

    glm::vec4 ToGlobalSpace(const glm::vec4 local) const
    {
        return matrix * local;
    }

    glm::vec3 ToLocalSpace(const glm::vec3 global) const
    {
        return glm::vec3(glm::inverse(matrix) * glm::vec4(global, 1.0));
    }
};
} // namespace yar
