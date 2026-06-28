#include "common.slang"

// Convert a direction to UV for use with
// equirectangular projection maps.
// Direction should be normalized.
float2 sphericalUV(float3 dir)
{
    const float2 polar = float2(atan2(-dir.y, dir.x), asin(-dir.z));
    const float2 invAtan = float2(0.5 / PI, 1.0 / PI);
    const float2 uv = (polar * invAtan) + 0.5;
    return uv;
}
