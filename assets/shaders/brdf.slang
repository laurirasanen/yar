#include "common.slang"

// Trowbridge-Reitz
float BRDFDistribution(float3 n, float3 h, float a)
{
    float a2 = a * a;
    float n_dot_h = dot(n, h);
    float n_dot_h2 = n_dot_h * n_dot_h;
    float denom = n_dot_h2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / denom;
}

float BRDFGeometrySchlick(float nom, float k)
{
    float denom = nom * (1.0 - k) + k;
    return nom / denom;
}

// Smith's Schlick
float BRDFGeometry(float3 n, float n_dot_v, float n_dot_l, float k)
{
    float sch1 = BRDFGeometrySchlick(n_dot_v, k);
    float sch2 = BRDFGeometrySchlick(n_dot_l, k);
    return sch1 * sch2;
}

// Fresnel-Schlick approximation
float3 BRDFFresnel(float cos_theta, float3 f)
{
    return f + (1.0 - f) * pow(1.0 - cos_theta, 5.0);
}

float3 BRDFFresnelRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 BRDF(
    float3 normal_dir,
    float3 light_dir,
    float3 view_dir,
    float3 half_dir,
    float3 albedo,
    float metalness,
    float roughness
)
{
    float n_dot_v = max(dot(normal_dir, view_dir), 0.0);
    float n_dot_l = max(dot(normal_dir, light_dir), 0.0);

    float3 dialectric = lerp(0.04, albedo, metalness);
    float3 fresnel = BRDFFresnel(max(dot(half_dir, view_dir), 0.0), dialectric);

    float ndf = BRDFDistribution(normal_dir, half_dir, roughness);

    float geo = BRDFGeometry(normal_dir, n_dot_v, n_dot_l, roughness);

    float3 nom = ndf * geo * fresnel;
    float denom = 4.0 * n_dot_v * n_dot_l + 0.0001;
    float3 specular = nom / denom;

    float3 k_d = (float3(1.0) - fresnel) * (1.0 - metalness);
    float3 diffuse = k_d * albedo / PI;

    return n_dot_l * (diffuse + specular);
}

float3 BRDF_IBL(
    GlobalData* globalData,
    float3 worldNormal,
    float3 normal,
    float3 worldView,
    float3 view,
    float3 albedo,
    float roughness,
    float metalness,
    float occlusion
)
{
    const float n_dot_v = max(dot(normal, view), 0.0);

    // Y-flip
    const float3 iblNormal = float3(worldNormal.x, worldNormal.z, -worldNormal.y);

    float3 iblDiffuse = texIBLFiltered[0].Sample(iblNormal).rgb
        * globalData->settings.w;

    float3 F0 = lerp(0.04, albedo.rgb, metalness);
    float3 fresnel = BRDFFresnelRoughness(n_dot_v, F0, roughness);
    float3 kD = 1.0 - fresnel;

    float3 diffuse = kD * iblDiffuse * albedo.rgb * occlusion;

    float3 specularDir = reflect(-worldView, worldNormal);
    specularDir = float3(specularDir.x, specularDir.z, -specularDir.y);

    const float specularMips = globalData->settings.z;
    const float specularLod = roughness * (specularMips - 1.0);
    float3 iblSpecular = texIBLFiltered[1].SampleLevel(specularDir, specularLod).rgb
        * globalData->settings.w;

    const float2 lutUV = float2(n_dot_v, roughness);
    const float2 iblLUT = texIBL[1].Sample(lutUV).rg;
    float3 scatter = fresnel * iblLUT.x + iblLUT.y;

    float3 specular = metalness * iblSpecular * scatter;

    return diffuse + specular;
}
