#include "common.slang"
#include "color.slang"

struct VertexOut
{
    float4 pos : SV_Position;
    float2 uv;
};

layout(binding = 0, set = 0)
Sampler2D texPreviousLevel;

layout(binding = 1, set = 0)
Sampler2D texDownsample;

[shader("vertex")]
VertexOut vertex(uint vertexID : SV_VertexID)
{
    VertexOut output;

    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * 2.0f + -1.0f, 0.0f, 1.0f);

    return output;
}

float3 Upsample( Sampler2D tex, float2 uv, float2 pixelSize )
{
    const float2 coords[9] = {
        float2( -1.0f,  1.0f ), float2(  0.0f,  1.0f ), float2(  1.0f,  1.0f ),
        float2( -1.0f,  0.0f ), float2(  0.0f,  0.0f ), float2(  1.0f,  0.0f ),
        float2( -1.0f, -1.0f ), float2(  0.0f, -1.0f ), float2(  1.0f, -1.0f )
    };

    const float weights[9] = {
        0.0625f, 0.125f, 0.0625f,
        0.125f,  0.25f,  0.125f,
        0.0625f, 0.125f, 0.0625f
    };

    float3 color = float3( 0.0f, 0.0f, 0.0f );

    for( int i = 0; i < 9; i++ )
    {
        float2 currentUV = uv + coords[i] * pixelSize;
        color += weights[i] * tex.SampleLevel(currentUV, 0).rgb;
    }

    return color;
}

[shader("pixel")]
PixelOut pixel(
    VertexOut input,
    uniform GlobalData* globalData
) {
    PixelOut output;

    const float radius = 0.85f;

    float3 downsampleColor = texDownsample.SampleLevel(input.uv, 0).rgb;

    float2 inputSize;
    texPreviousLevel.GetDimensions(inputSize.x, inputSize.y);
    float2 pixelSize = 1.0 / inputSize;
    float3 previousColor = Upsample(texPreviousLevel, input.uv, pixelSize);

    output.color.rgb = lerp(downsampleColor, previousColor, radius);
    output.color.a = 1.0;

    return output;
}
