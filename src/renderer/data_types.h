#pragma once

#include <cstdint>
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

#include "../components/camera.h"
#include "../log.h"

namespace yar
{
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 color;
};

typedef uint32_t Index;

struct ShaderGlobalData
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 viewProj;

    alignas(16) glm::mat4 invView;
    alignas(16) glm::mat4 invProj;
    alignas(16) glm::mat4 invViewProj;

    alignas(16) glm::vec4 viewport;

    alignas(16) glm::vec3 eye;

    ShaderGlobalData(const std::shared_ptr<Camera> cam)
    {
        view     = cam->view;
        proj     = cam->proj;
        viewProj = proj * view;

        invView     = glm::inverse(view);
        invProj     = glm::inverse(proj);
        invViewProj = glm::inverse(proj * view);

        viewport = cam->viewport;

        eye = cam->transform.position;
    }
};

struct ShaderObjectData
{
    alignas(16) glm::mat4 model;
    alignas(16) uint32_t materialID;
};
} // namespace yar
