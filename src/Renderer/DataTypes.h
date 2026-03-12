#pragma once

#include <memory>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>

#include "../Components/Camera.h"
#include "../Log.h"

namespace drive
{
struct Vertex_P
{
    glm::vec3 position;
};

struct Vertex_P_C
{
    glm::vec3 position;
    glm::vec3 color;
};

struct Vertex_P_N_C
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

// For fullscreen triangle
struct VertexEmpty
{
};

typedef uint32_t Index;

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 mvp;
    alignas(16) glm::mat4 invModel;
    alignas(16) glm::mat4 invView;
    alignas(16) glm::mat4 invProj;
    alignas(16) glm::mat4 clipToWorld;
    alignas(16) glm::vec3 eye;

    alignas(16) glm::vec4 viewport;

    alignas(16) glm::vec3 sunDir;
    alignas(16) glm::vec3 sunColor;

    UniformBufferObject(const std::shared_ptr<Camera> cam)
    {
        model       = glm::identity<glm::mat4>();
        view        = cam->view;
        proj        = cam->proj;
        mvp         = proj * view * model;
        invModel    = glm::inverse(model);
        invView     = glm::inverse(view);
        invProj     = glm::inverse(proj);
        clipToWorld = glm::inverse(proj * view);
        eye         = cam->transform.position;

        viewport = cam->viewport;

        const auto degreesPerSecond = 0.5;
        auto       sunRotation      = glm::angleAxis(
            glm::radians(degreesPerSecond * Time::Uptime()),
            glm::normalize(glm::dvec3 {1.0, 0.3, 0.2})
        );
        sunDir   = glm::normalize(sunRotation * glm::dvec3(0.1, 0.2, 1.0));
        sunColor = glm::vec3(0.5, 0.5, 0.5);
    }
};
} // namespace drive
