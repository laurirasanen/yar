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
    glm::mat4 matrix;

    void SetPosition(glm::vec3 pos)
    {
        matrix[3][0] = pos.x;
        matrix[3][1] = pos.x;
        matrix[3][2] = pos.x;
    }

    glm::vec3 GetPosition() const
    {
        return glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
    }

    glm::vec3 SetScale(glm::vec3 scale)
    {
        matrix[0][0] = scale.x;
        matrix[1][1] = scale.x;
        matrix[2][2] = scale.x;
    }

    glm::vec3 GetScale() const
    {
        return glm::vec3(matrix[0][0], matrix[1][1], matrix[2][2]);
    }

    glm::vec3 GetEulerRotation() const
    {
        // todo
        return glm::vec3 {};
    }

    void SetEulerRotation(const glm::vec3 angles)
    {
        // todo
    }

    glm::quat GetRotation() const
    {
        // todo
        return glm::identity<glm::quat>();
    }

    void SetRotation(glm::quat rotation)
    {
        // todo
    }
};
} // namespace yar
