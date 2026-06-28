#pragma once

#include <cstdint>
#include <cstring>
#include <memory>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>

#include "../components/camera.h"

namespace yar
{
enum VertexType
{
    Unlit,
    Shaded,
};

struct VertexUnlit
{
    glm::vec3 position;
    glm::vec3 color;
};

struct VertexShaded
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv;
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

    alignas(16) glm::vec4 eye;

    alignas(16) glm::vec4 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec4 ambientLight;

    alignas(16) glm::vec4 iblDiffuse;
    alignas(16) glm::vec4 iblSpecular;

    alignas(16) glm::vec4 settings;

    ShaderGlobalData()
    {
        lightDir     = glm::vec4(glm::normalize(glm::vec3(-0.3f, -0.45f, 0.6f)), 0);
        lightColor   = glm::vec4(1.0f, 1.0f, 0.9f, 15.0f);
        ambientLight = glm::vec4(0.6f, 0.85f, 1.0f, 0.05f);
        iblDiffuse   = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        iblSpecular  = glm::vec4(1.0f, 1.0f, 1.0f, 2.0f);
        SetExposure(1.20f);
        SetContrast(1.15f);
    }

    void Update(const std::shared_ptr<Camera> cam)
    {
        view     = cam->view;
        proj     = cam->proj;
        viewProj = proj * view;

        invView     = glm::inverse(view);
        invProj     = glm::inverse(proj);
        invViewProj = glm::inverse(viewProj);

        viewport = cam->viewport;

        eye = glm::vec4(cam->transform.GetPosition(), 0);
    }

    void SetExposure(float exposure)
    {
        settings[0] = exposure;
    }

    float GetExposure()
    {
        return settings[0];
    }

    void SetContrast(float contrast)
    {
        settings[1] = contrast;
    }

    float GetContrast()
    {
        return settings[1];
    }

    void SetIBLMips(float max)
    {
        settings[2] = max;
    }
};

struct ShaderObjectData
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 normal;
    alignas(16) uint32_t index;
    alignas(16) glm::vec4 materialParams;
    alignas(16) glm::vec4 materialParams2;
};
} // namespace yar
