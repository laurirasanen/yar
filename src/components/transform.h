#pragma once

#include <cmath>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
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

    void SetScale(glm::vec3 scale)
    {
        matrix[0][0] = scale.x;
        matrix[1][1] = scale.y;
        matrix[2][2] = scale.z;
    }

    glm::vec3 GetScale() const
    {
        return glm::vec3(matrix[0][0], matrix[1][1], matrix[2][2]);
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

    glm::vec3 Forward()
    {
        return GetRotation() * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::vec3 Right()
    {
        return GetRotation() * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 Up()
    {
        return GetRotation() * glm::vec3(0.0f, 0.0f, 1.0f);
    }
};
} // namespace yar
