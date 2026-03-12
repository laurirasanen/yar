#version 450

#include "include/VertexPNC.glsl"
#include "include/UniformBufferObject.glsl"
#include "include/Lighting.glsl"

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 fragNormal;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragPos = (ubo.model * vec4(inPosition, 1.0)).xyz;
    fragColor = inColor;
    fragNormal = (ubo.model * vec4(inNormal, 1.0)).xyz;
}
