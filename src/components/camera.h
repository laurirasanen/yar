#pragma once

#include <algorithm>
#include <cfloat>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/matrix.hpp>

#include "../time.h"
#include "../window/input.h"
#include "geometry.h"
#include "transform.h"

namespace yar
{
#define CAM_NEAR 0.1f
#define CAM_FAR  1000.0f

class Camera
{
  public:
    Camera()
    {
        transform = {};
        fov       = 60.0f;
        near      = CAM_NEAR;
        far       = CAM_FAR;
        UpdateViewport(1920.0f, 1080.0f);
        UpdateMatrices();
        UpdateFrustum();
    }

    virtual ~Camera() {};

    void UpdateMatrices()
    {
        const auto pos = transform.GetPosition();
        view           = glm::lookAt(pos, pos + transform.Forward(), glm::vec3(0.0f, 0.0f, 1.0f));
        proj           = glm::perspective(glm::radians(fov), aspect, near, far);
        proj[1][1] *= -1; // OpenGL Y flip
    }

    void UpdateViewport(int width, int height)
    {
        aspect     = static_cast<float>(width) / static_cast<float>(height);
        viewport.x = static_cast<float>(width);
        viewport.y = static_cast<float>(height);
        viewport.z = near;
        viewport.w = far;
    }

    void UpdateFrustum()
    {
        const auto trans    = glm::transpose(proj * view);
        frustum.left.clip   = trans[3] + trans[0];
        frustum.right.clip  = trans[3] - trans[0];
        frustum.bottom.clip = trans[3] + trans[1];
        frustum.top.clip    = trans[3] - trans[1];
        frustum.near.clip   = trans[3] + trans[2];
        frustum.far.clip    = trans[3] - trans[2];
    }

    bool IsInFrustum(const glm::vec3 pos, float margin = 0.0f)
    {
        // clang-format off
        if (frustum.left.SignedDistance(pos)   < margin) return false;
        if (frustum.right.SignedDistance(pos)  < margin) return false;
        if (frustum.far.SignedDistance(pos)    < margin) return false;
        if (frustum.near.SignedDistance(pos)   < margin) return false;
        if (frustum.top.SignedDistance(pos)    < margin) return false;
        if (frustum.bottom.SignedDistance(pos) < margin) return false;
        // clang-format on

        return true;
    }

    bool IsInFrustum(const Sphere& sphere)
    {
        return IsInFrustum(sphere.center, -sphere.radius);
    }

    // special case for AABB since it may cover the whole frustum,
    // with all corners being outside it.
    bool IsInFrustum(const AABB& aabb)
    {
        const FrustumPlane planes[] = {
            frustum.left,
            frustum.right,
            frustum.far,
            frustum.near,
            frustum.top,
            frustum.bottom,
        };
        const glm::vec3 corners[] = {
            glm::vec3(aabb.min.x, aabb.min.y, aabb.min.z),
            glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z),
            glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z),
            glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z),
            glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z),
            glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z),
            glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z),
            glm::vec3(aabb.max.x, aabb.max.y, aabb.max.z),
        };

        for (const auto& plane : planes)
        {
            // clang-format off
            if (plane.SignedDistance(corners[0]) < 0 &&
                plane.SignedDistance(corners[1]) < 0 &&
                plane.SignedDistance(corners[2]) < 0 &&
                plane.SignedDistance(corners[3]) < 0 &&
                plane.SignedDistance(corners[4]) < 0 &&
                plane.SignedDistance(corners[5]) < 0 &&
                plane.SignedDistance(corners[6]) < 0 &&
                plane.SignedDistance(corners[7]) < 0
            )
            {
                return false;
            }
            //clang-format on
        }

        return true;
    }

    float GetDistance(const AABB& aabb)
    {
        const glm::vec3 corners[] = {
            glm::vec3(aabb.min.x, aabb.min.y, aabb.min.z),
            glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z),
            glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z),
            glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z),
            glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z),
            glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z),
            glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z),
            glm::vec3(aabb.max.x, aabb.max.y, aabb.max.z),
        };
        float distance = FLT_MAX;
        for (const auto& corner : corners)
        {
            distance = std::min(distance, glm::length(corner - transform.GetPosition()));
        }
        return distance;
    }

    virtual void HandleInput(WindowInput) {};

    Transform transform;
    float     fov;
    float     aspect;
    float     near;
    float     far;
    glm::vec4 viewport;
    glm::mat4 view;
    glm::mat4 proj;
    Frustum   frustum;
};

class NoclipCamera : public Camera
{
  public:
    NoclipCamera() : Camera()
    {
        MoveSpeed   = 1.0f;
        Sensitivity = 2.0f;
    }

    void HandleInput(WindowInput input) override
    {
        auto euler = transform.GetEulerRotation();
        if (input.mouse.x != 0)
        {
            euler.z -= 0.022f * 3.14f * static_cast<float>(input.mouse.x);
            while (euler.z < -180.0f)
            {
                euler.z += 360.0f;
            }
            while (euler.z > 180.0f)
            {
                euler.z -= 360.0f;
            }
        }
        if (input.mouse.y != 0)
        {
            euler.x -= 0.022f * 3.14f * static_cast<float>(input.mouse.y);
            euler.x = std::clamp(euler.x, -89.0f, 89.0f);
        }
        transform.SetEulerRotation(euler);

        if (input.scroll.y > 0)
        {
            MoveSpeed *= 2;
        }
        else if (input.scroll.y < 0)
        {
            MoveSpeed /= 2;
        }

        auto position = transform.GetPosition();
        if (input.HasKey(Key::KEY_MOVE_FORWARD))
        {
            position += transform.Forward() * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_BACK))
        {
            position -= transform.Forward() * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_RIGHT))
        {
            position += transform.Right() * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_LEFT))
        {
            position -= transform.Right() * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_UP))
        {
            position += glm::vec3(0, 0, 1) * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_DOWN))
        {
            position -= glm::vec3(0, 0, 1) * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        transform.SetPosition(position);

        UpdateMatrices();
        UpdateFrustum();
    }

    float MoveSpeed;
    float Sensitivity;
};
} // namespace yar
