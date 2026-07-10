#include "bindings.slang"
#include "common.slang"
#include "math.slang"

struct VertexOutSky
{
    float4 pos : SV_Position;
    float2 uv;
};

[shader("vertex")]
VertexOutSky vertex(
    uint vertexID : SV_VertexID
) {
    VertexOutSky output;

    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * 2.0f + -1.0f, 1.0 - 1e-5, 1.0f);

    return output;
}

[shader("pixel")]
PixelOut pixel(
    VertexOutSky input,
    uniform GlobalData* globalData,
) {
    PixelOut output;

    const float4 clipPos = FragClipPos(input.pos, globalData->viewport.xy);
    const float3 viewPos = mul(clipPos, globalData->invProj).xyz;
    const float3 worldDir = normalize(mul(viewPos, float3x3(globalData->invView)));

    float2 uv = SphericalUV(worldDir);
    uv.y = 1.0 -uv.y;
    float3 color = texIBL[0].SampleLevel(uv, 0).rgb * globalData->settings.w;

    output.color = float4(color, 1.0);

    return output;
}
