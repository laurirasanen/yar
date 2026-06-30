#ifndef COMMON_SLANG
#define COMMON_SLANG

#define MAT_MVP (mul(objectData.model, globalData->viewProj))

#define PI 3.14159265358979323846

struct ObjectData
{
    float4x4 model;
    float4x4 normal;
    uint index;
    // x = occlusion, y = roughness, z = metalness, w = unused
    float4 materialParams;
    // xyz = emissive factor, w = unused
    float4 materialParams2;
};

layout(binding = 0, set = 0)
ConstantBuffer<ObjectData> objectData;

layout(binding = 1, set = 0)
Sampler2D texAlbedo[];

layout(binding = 2, set = 0)
Sampler2D texMRAO[];

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

struct GlobalData
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;

    float4x4 invView;
    float4x4 invProj;
    float4x4 invViewProj;

    float4 viewport;

    float4 eye;

    float4 lightDir;
    float4 lightColor;
    float4 ambientLight;

    // exposure, contrast, ibl specular mips, ibl strength
    float4 settings;
};

struct VertexSky
{
    float2 pos : POSITION;
};

struct VertexOutSky
{
    float4 pos : SV_Position;
};

struct VertexUnlit
{
    float3 pos    : POSITION;
    float3 color;
};

struct VertexOutUnlit
{
    float4 pos : SV_Position;
    float3 color;
};

struct VertexShaded
{
    float3 pos       : POSITION;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float2 uv        : TEXCOORD;
};

struct VertexOutShaded
{
    float4 pos  : SV_Position;
    float2 uv   : TEXCOORD;
    float3 normal;
    float3 viewDir;
    float3 tangentPos;
    float3 tangentLightDir;
    float3 tangentEye;
};

struct PixelOut
{
    float4 color : SV_Target0;
};

#endif // COMMON_SLANG
