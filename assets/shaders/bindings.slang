#ifndef BINDINGS_SLANG
#define BINDINGS_SLANG

#define MAT_MVP (mul(objectData.model, globalData->viewProj))

#define UINT32_MAX 4294967295

#define TEX_ALBEDO(uv)   (objectData.texIndices.x < UINT32_MAX ? texAlbedo[objectData.texIndices.x].SampleLevel(0, uv) : float4(1.0, 0, 1.0, 1.0))
#define TEX_ORM(uv)      (objectData.texIndices.y < UINT32_MAX ? texORM[objectData.texIndices.y].SampleLevel(0, uv).rgb : float3(0, 0.8, 0))
#define TEX_NORMAL(uv)   (objectData.texIndices.z < UINT32_MAX ? texNormal[objectData.texIndices.z].SampleLevel(0, uv).rgb : float3(0.5, 0.5, 1.0))
#define TEX_EMISSIVE(uv) (objectData.texIndices.w < UINT32_MAX ? texEmissive[objectData.texIndices.w].SampleLevel(0, uv).rgb : float3(0, 0, 0))

struct ObjectData
{
    float4x4 model;
    float4x4 normal;
    // albedo, orm, normal, emissive
    uint4 texIndices;
    // occlusion, roughness, metalness, unused
    float4 materialParams;
    // xyz = emissive factor, unused
    float4 materialParams2;
};

layout(binding = 0, set = 0)
ConstantBuffer<ObjectData> objectData;

layout(binding = 1, set = 0)
Sampler2D texAlbedo[];

layout(binding = 2, set = 0)
Sampler2D texORM[];

layout(binding = 3, set = 0)
Sampler2D texNormal[];

layout(binding = 4, set = 0)
Sampler2D texEmissive[];

// Color, LUT
layout(binding = 5, set = 0)
Sampler2D texIBL[];

// diffuse, specular
layout(binding = 6, set = 0)
SamplerCube texIBLFiltered[];

#endif // BINDINGS_SLANG
