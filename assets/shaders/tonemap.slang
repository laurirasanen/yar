#include "common.slang"
#include "color.slang"

struct VertexOut
{
    float4 pos : SV_Position;
    float2 uv;
};

layout(binding = 0, set = 0)
Sampler2D texColor;

layout(binding = 1, set = 0)
Sampler2D texDepth;

[shader("vertex")]
VertexOut vertex(uint vertexID : SV_VertexID)
{
    VertexOut output;

    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * 2.0f + -1.0f, 0.0f, 1.0f);

    return output;
}

[shader("pixel")]
PixelOut pixel(
    VertexOut input,
    uniform GlobalData* globalData
) {
    PixelOut output;

    float3 color = texColor.Sample(input.uv).rgb;

    const float exposure = globalData->settings[0];
    color = color * exposure;
    color = ACESTonemap(color);
    const float contrast = globalData->settings[1];
    color = pow(color, contrast);

    output.color = float4(color, 1.0);

    return output;
}
