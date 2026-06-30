#include "common.slang"

// Convert a direction to UV for use with
// equirectangular projection maps.
// Direction should be normalized.
float2 SphericalUV(float3 dir)
{
    const float2 polar = float2(atan2(dir.z, dir.x), asin(dir.y));
    const float2 invAtan = float2(0.5 / PI, 1.0 / PI);
    const float2 uv = (polar * invAtan) + 0.5;
    return uv;
}

float4 FragClipPos(float4 pos, float2 viewport)
{
    float4 ndc;
    ndc.xy      = (2.0 * pos.xy / viewport) - 1.0;
    ndc.z       = (2.0 * pos.z) - 1.0;
    ndc.w       = 1.0;
    float4 clip = ndc / pos.w;
    return clip;
}

float3 FragWorldPos(float4 pos, float2 viewport, float4x4 invViewProj)
{
    return mul(FragClipPos(pos, viewport), invViewProj).xyz;
}
