#include "common.slang"
#include "color.slang"

struct VertexOut
{
    float4 pos : SV_Position;
    float2 uv;
};

layout(binding = 0, set = 0)
Sampler2D texColor;

[shader("vertex")]
VertexOut vertex(uint vertexID : SV_VertexID)
{
    VertexOut output;

    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * 2.0f + -1.0f, 0.0f, 1.0f);

    return output;
}

float3 Downsample(Sampler2D tex, float2 uv, float2 pixelSize)
{
    const float2 coords[13] = {
        float2( -1.0f,  1.0f ), float2(  1.0f,  1.0f ),
        float2( -1.0f, -1.0f ), float2(  1.0f, -1.0f ),

        float2( -2.0f,  2.0f ), float2( 0.0f, 2.0f ), float2( 2.0f, 2.0f ),
        float2( -2.0f,  0.0f ), float2( 0.0f, 0.0f ), float2( 2.0f, 0.0f ),
        float2( -2.0f, -2.0f ), float2( 0.0f,-2.0f ), float2( 2.0f,-2.0f )
    };


    const float weights[13] = {
        // 4 samples
        // (1 / 4) * 0.5f = 0.125f
        0.125f, 0.125f,
        0.125f, 0.125f,

        // 9 samples
        // (1 / 9) * 0.5f
        0.0555555f, 0.0555555f, 0.0555555f,
        0.0555555f, 0.0555555f, 0.0555555f,
        0.0555555f, 0.0555555f, 0.0555555f
    };

    float3 outColor = float3( 0.0f, 0.0f, 0.0f );

    for( int i = 0; i < 13; i++ )
    {
        float2 currentUV = uv + coords[i] * pixelSize;
        outColor += weights[i] * tex.SampleLevel(currentUV, 0).rgb;
    }

    return outColor;
}

[shader("pixel")]
PixelOut pixel(
    VertexOut input,
    uniform GlobalData* globalData
) {
    PixelOut output;

    float2 inputSize;
    texColor.GetDimensions(inputSize.x, inputSize.y);
    float2 pixelSize = 0.5 / inputSize;
    output.color.rgb = Downsample(texColor, input.uv, pixelSize);
    output.color.a = 1.0;

    return output;
}
