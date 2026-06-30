#include "common.slang"
#include "math.slang"
#include "color.slang"

[shader("vertex")]
VertexOutSky vertex(
    VertexSky           input,
    uniform GlobalData* globalData,
) {
    VertexOutSky output;

    output.pos = float4(input.pos, 0.99, 1.0);

    const float3 viewPos = mul(output.pos, globalData->invProj).xyz;
    const float3 worldDir = normalize(mul(viewPos, float3x3(globalData->invView)));
    output.viewDir = worldDir;

    return output;
}

[shader("pixel")]
PixelOut pixel(
    VertexOutSky input,
    uniform GlobalData* globalData,
) {
    PixelOut output;

    const float2 uv = sphericalUV(normalize(input.viewDir));
    float3 color = texIBL[0].Sample(uv).rgb * globalData->settings.w;

    const float exposure = globalData->settings[0];
    color = color * exposure;
    color = ACESTonemap(color);
    const float contrast = globalData->settings[1];
    color = pow(color, contrast);

    output.color = float4(color, 1.0);

    return output;
}
