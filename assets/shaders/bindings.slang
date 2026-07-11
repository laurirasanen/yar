#ifndef BINDINGS_SLANG
#define BINDINGS_SLANG

#define MAT_MVP (mul(objectData.model, globalData->viewProj))

#define UINT32_MAX 4294967295

float4 SampleAlbedo(float2 uv)
{
    const uint matIdx = objectData.params[0];
    const float4 missingAlbedo = float4(1.0, 0, 1.0, 1.0);

    if (matIdx >= UINT32_MAX)
    {
        return missingAlbedo;
    }

    const uint firstTexIdx = materials[matIdx].params[0];
    const uint texCount = materials[matIdx].params[1];
    if (texCount < 1)
    {
        return missingAlbedo;
    }

    return textures[firstTexIdx].SampleLevel(uv, 0).rgba;
}

float3 SampleORM(float2 uv)
{
    const uint matIdx = objectData.params[0];
    const float3 missingORM = float3(0, 0.8, 0);

    if (matIdx >= UINT32_MAX)
    {
        return missingORM;
    }

    const uint firstTexIdx = materials[matIdx].params[0];
    const uint texCount = materials[matIdx].params[1];
    if (texCount < 2)
    {
        return missingORM;
    }

    float3 texVal = textures[firstTexIdx + 1].SampleLevel(uv, 0).rgb;

    const uint firstParamIdx = materials[matIdx].params[2];
    if (firstParamIdx < UINT32_MAX)
    {
        const uint paramCount = materials[matIdx].params[3];
        if (paramCount > 2)
        {
            const float occlusion = parameters[firstParamIdx];
            const float roughness = parameters[firstParamIdx + 1];
            const float metallic  = parameters[firstParamIdx + 2];
            texVal.r  = pow(texVal.r, occlusion);
            texVal.g *= roughness;
            texVal.b *= metallic;
        }
    }

    return texVal;
}

float3 SampleNormal(float2 uv)
{
    const uint matIdx = objectData.params[0];
    const float3 missingNormal = float3(0.5, 0.5, 1.0);

    if (matIdx >= UINT32_MAX)
    {
        return missingNormal;
    }

    const uint firstTexIdx = materials[matIdx].params[0];
    const uint texCount = materials[matIdx].params[1];
    if (texCount < 3)
    {
        return missingNormal;
    }

    return textures[firstTexIdx + 2].SampleLevel(uv, 0).rgb;
}

float3 SampleEmissive(float2 uv)
{
    const uint matIdx = objectData.params[0];
    const float3 missingEmissive = float3(0.0, 0.0, 0.0);

    if (matIdx >= UINT32_MAX)
    {
        return missingEmissive;
    }

    const uint firstTexIdx = materials[matIdx].params[0];
    const uint texCount = materials[matIdx].params[1];
    if (texCount < 4)
    {
        return missingEmissive;
    }

    float3 texVal = textures[firstTexIdx + 3].SampleLevel(uv, 0).rgb;

    const uint firstParamIdx = materials[matIdx].params[2];
    if (firstParamIdx < UINT32_MAX)
    {
        const uint paramCount = materials[matIdx].params[3];
        if (paramCount > 5)
        {
            texVal.r *= parameters[firstParamIdx + 3];
            texVal.g *= parameters[firstParamIdx + 4];
            texVal.b *= parameters[firstParamIdx + 5];
        }
    }

    return texVal;
}

struct ObjectData
{
    float4x4 model;
    float4x4 normal;
    // material, unused
    uint4 params;
};

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 uv;
};

struct Material
{
    // first texture, count, first param, count
    uint4 params;
};

layout(binding = 0, set = 0)
ConstantBuffer<ObjectData> objectData;

layout(binding = 1, set = 0)
ConstantBuffer<Vertex[]> vertices;

layout(binding = 2, set = 0)
ConstantBuffer<Material[]> materials;

layout(binding = 3, set = 0)
ConstantBuffer<float[]> parameters;

layout(binding = 4, set = 0)
Sampler2D textures[];

// Color, LUT
layout(binding = 5, set = 0)
Sampler2D texIBL[];

// diffuse, specular
layout(binding = 6, set = 0)
SamplerCube texIBLFiltered[];

#endif // BINDINGS_SLANG
