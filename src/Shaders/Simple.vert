#version 450

#include "include/VertexPC.glsl"
#include "include/UniformBufferObject.glsl"

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}
