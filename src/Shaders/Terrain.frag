#version 450

#include "include/UniformBufferObject.glsl"
#include "include/Lighting.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 light = BlinnPhong(fragPos, fragNormal, ubo.eye, 1.0);
    outColor = vec4(light * fragColor, 1.0);
}
