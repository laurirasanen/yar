#pragma once

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
    void SetPosition(glm::vec3 pos)
    {
        m_model[3][0] = pos.x;
        m_model[3][1] = pos.y;
        m_model[3][2] = pos.z;
    }

    glm::vec3 GetPosition() const
    {
        return glm::vec3(m_model[3][0], m_model[3][1], m_model[3][2]);
    }

    void Scale(const glm::vec3 scale)
    {
        m_model[0] *= scale.x;
        m_model[1] *= scale.y;
        m_model[2] *= scale.z;
    }

    void SetScale(const glm::vec3 scale)
    {
        Scale(scale / GetScale());
    }

    glm::vec3 GetScale() const
    {
        return glm::vec3(
            glm::length(glm::vec3(m_model[0])),
            glm::length(glm::vec3(m_model[1])),
            glm::length(glm::vec3(m_model[2]))
        );
    }

    glm::mat4 GetModelMatrix() const
    {
        return m_model;
    }

    void SetModelMatrix(const glm::mat4& mat)
    {
        m_model = mat;
    }

    void CopyFrom(const Transform& other)
    {
        SetModelMatrix(other.GetModelMatrix());
        SetRotation(other.GetRotation());
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
        return m_rotation;
    }

    glm::mat4 GetRotationMatrix() const
    {
        return glm::mat4_cast(m_rotation);
    }

    glm::mat4 GetCombinedMatrix() const
    {
        return GetModelMatrix() * GetRotationMatrix();
    }

    void SetRotation(const glm::quat rotation)
    {
        m_rotation = rotation;
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
        return glm::vec3(GetCombinedMatrix() * glm::vec4(local, 1.0));
    }

    glm::vec4 ToGlobalSpace(const glm::vec4 local) const
    {
        return GetCombinedMatrix() * local;
    }

    glm::vec3 ToLocalSpace(const glm::vec3 global) const
    {
        return glm::vec3(glm::inverse(GetCombinedMatrix()) * glm::vec4(global, 1.0));
    }

  private:
    glm::mat4 m_model    = glm::identity<glm::mat4>();
    glm::quat m_rotation = glm::identity<glm::quat>();
};
} // namespace yar
