#pragma once

#include <algorithm>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtc/constants.hpp>

#include "../time.h"
#include "../window/input.h"
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
        transform.SetPosition(glm::vec3(0, -3.0, 1.0));
        fov  = 60.0f;
        near = CAM_NEAR;
        far  = CAM_FAR;
        UpdateViewport(1920.0f, 1080.0f);
        UpdateMatrices();
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
        viewport.z = CAM_NEAR;
        viewport.w = CAM_FAR;
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
    }

    float MoveSpeed;
    float Sensitivity;
};
} // namespace yar
