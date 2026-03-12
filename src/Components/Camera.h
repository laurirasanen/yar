#pragma once

#include <algorithm>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtc/constants.hpp>

#include "../Time.h"
#include "../Window/Input.h"
#include "Transform.h"

namespace drive
{
#define CAM_NEAR 0.1f
#define CAM_FAR  1000.0f

class Camera
{
  public:
    Camera()
    {
        transform          = {};
        transform.position = glm::vec3(0, -3.0, 100.0);
        fov                = 60.0f;
        near               = CAM_NEAR;
        far                = CAM_FAR;
        UpdateViewport(1920.0f, 1080.0f);
        UpdateMatrices();
    }

    virtual ~Camera() {};

    glm::vec3 Forward()
    {
        return transform.rotation.quaternion * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::vec3 Right()
    {
        return transform.rotation.quaternion * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 Up()
    {
        return transform.rotation.quaternion * glm::vec3(0.0f, 0.0f, 1.0f);
    }

    void UpdateMatrices()
    {
        transform.rotation.quaternion =
            glm::angleAxis(glm::radians(transform.rotation.euler.z), glm::vec3 {0.0f, 0.0f, 1.0f})
            * glm::angleAxis(
                glm::radians(transform.rotation.euler.x),
                glm::vec3 {1.0f, 0.0f, 0.0f}
            );
        view = glm::lookAt(
            transform.position,
            transform.position + Forward(),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        proj = glm::perspective(glm::radians(fov), aspect, near, far);
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
        if (input.mouse.x != 0)
        {
            transform.rotation.euler.z -= 0.022f * 3.14f * static_cast<float>(input.mouse.x);
            while (transform.rotation.euler.z < -180.0f)
            {
                transform.rotation.euler.z += 360.0f;
            }
            while (transform.rotation.euler.z > 180.0f)
            {
                transform.rotation.euler.z -= 360.0f;
            }
        }
        if (input.mouse.y != 0)
        {
            transform.rotation.euler.x -= 0.022f * 3.14f * static_cast<float>(input.mouse.y);
            transform.rotation.euler.x = std::clamp(transform.rotation.euler.x, -89.0f, 89.0f);
        }

        if (input.scroll.y > 0)
        {
            MoveSpeed *= 2;
        }
        else if (input.scroll.y < 0)
        {
            MoveSpeed /= 2;
        }

        if (input.HasKey(Key::KEY_MOVE_FORWARD))
        {
            transform.position += Forward() * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_BACK))
        {
            transform.position -= Forward() * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_RIGHT))
        {
            transform.position += Right() * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_LEFT))
        {
            transform.position -= Right() * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_UP))
        {
            transform.position +=
                glm::vec3(0, 0, 1) * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }
        if (input.HasKey(Key::KEY_MOVE_DOWN))
        {
            transform.position -=
                glm::vec3(0, 0, 1) * MoveSpeed * static_cast<float>(Time::DeltaFrame);
        }

        UpdateMatrices();
    }

    float MoveSpeed;
    float Sensitivity;
};
} // namespace drive
