#version 450

#include "include/VertexP.glsl"
#include "include/UniformBufferObject.glsl"

layout(location = 0) out vec3 fragDir;

void main()
{
    fragDir = normalize((transpose(ubo.view) * vec4(inPosition, 1.0)).xyz);
    gl_Position = ubo.proj * vec4(inPosition, 1.0);
}
