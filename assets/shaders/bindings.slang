#ifndef BINDINGS_SLANG
#define BINDINGS_SLANG

#define MAT_MVP (mul(objectData.model, globalData->viewProj))

#define TEX_ALBEDO   (  texAlbedo[objectData.texIndices.x])
#define TEX_ORM      (     texORM[objectData.texIndices.y])
#define TEX_NORMAL   (  texNormal[objectData.texIndices.z])
#define TEX_EMISSIVE (texEmissive[objectData.texIndices.w])

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
