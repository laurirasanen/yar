#include "common.slang"

float D_GGX(float roughness, float NoH, float3 N, float3 H)
{
    const float3 NxH = cross(N, H);
    const float a = NoH * roughness;
    const float k = roughness / (dot(NxH, NxH) + a * a);
    return k * k * (1.0 / PI);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

float V_SmithGGXCorrelatedFast(float NoV, float NoL, float roughness)
{
    float GGXV = NoL * (NoV * (1.0 - roughness) + roughness);
    float GGXL = NoV * (NoL * (1.0 - roughness) + roughness);
    return 0.5 / (GGXV + GGXL);
}

float3 F_Schlick(float u, float3 f0, float3 f90)
{
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

float F_Schlick(float u, float f0, float f90)
{
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

float F_Schlick(float u, float f0)
{
    return F_Schlick(u, f0, 1.0);
}

float3 F_Schlick(float u, float3 f0)
{
    return F_Schlick(u, f0, 1.0);
}

float Fd_Burley(float NoV, float NoL, float LoH, float roughness) {
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = F_Schlick(NoL, 1.0, f90);
    float viewScatter = F_Schlick(NoV, 1.0, f90);
    return lightScatter * viewScatter * (1.0 / PI);
}

float3 BRDF(
    float3 N,
    float3 L,
    float3 V,
    float3 H,
    float3 basecolor,
    float metallic,
    float perceptualRoughness
)
{
    const float roughness = perceptualRoughness * perceptualRoughness;

    const float NoV = max(dot(N, V), 0.0) + 1e-5;
    const float NoL = clamp(dot(N, L), 0.0, 1.0);
    const float LoH = clamp(dot(L, H), 0.0, 1.0);
    const float NoH = clamp(dot(N, H), 0.0, 1.0);

    float D = D_GGX(NoH, roughness, N, H);
    float f0 = 0.04;
    float3 F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelatedFast(NoV, NoL, roughness);

    float3 Fr = (D * V) * F;

    const float3 diffuseColor = (1.0 - metallic) * basecolor.rgb;
    float3 Fd = diffuseColor * Fd_Burley(NoV, NoL, LoH, roughness);

    return Fd + Fr;
}

float3 BRDF_IBL(
    GlobalData* globalData,
    float3 worldNormal,
    float3 N,
    float3 worldView,
    float3 V,
    float3 basecolor,
    float perceptualRoughness,
    float metallic,
    float occlusion
)
{
    const float roughness = perceptualRoughness * perceptualRoughness;

    const float3 L = normalize(reflect(-worldView, worldNormal));
    const float3 H = normalize(worldView + L);

    const float NoV = max(dot(N, V), 0.0);
    const float LoH = clamp(dot(L, H), 0.0, 1.0);

    const float specularMips = globalData->settings.z;
    const float specularLod = roughness * (specularMips - 1.0);
    float3 D = texIBLFiltered[1].SampleLevel(L, specularLod).rgb
        * globalData->settings.w;

    float3 iblDiffuse = texIBLFiltered[0].Sample(worldNormal).rgb
        * globalData->settings.w;

    float3 f0 = lerp(0.04, basecolor.rgb, metallic);
    float f90 = clamp(0.5 + 2.0 * roughness * LoH * LoH, 0.0, 1.0);
    float3 F = F_Schlick(LoH, f0, f90);

    const float3 diffuseColor = (1.0 - metallic) * basecolor.rgb;
    float3 kD = (1.0 - F) * (1.0 - metallic);
    float3 Fd = kD * iblDiffuse * diffuseColor * occlusion;

    const float2 lutUV = float2(NoV, perceptualRoughness);
    const float2 visLUT = texIBL[1].Sample(lutUV).rg;
    float3 FV = F * visLUT.x + visLUT.y;

    float3 Fr = f0 * D * FV;

    return Fd + Fr;
}
