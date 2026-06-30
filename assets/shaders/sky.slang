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
    // lod has to be explicit here to avoid seams
    float3 color = texIBL[0].SampleLevel(uv, 0).rgb * globalData->settings.w;

    const float exposure = globalData->settings[0];
    color = color * exposure;
    color = ACESTonemap(color);
    const float contrast = globalData->settings[1];
    color = pow(color, contrast);

    output.color = float4(color, 1.0);

    return output;
}
