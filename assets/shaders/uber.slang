#include "bindings.slang"
#include "common.slang"
#include "math.slang"
#include "brdf.slang"

struct VertexOut
{
    float4 pos  : SV_Position;
    float2 uv   : TEXCOORD;
    float3 normal;
    float3 viewDir;
    float3 tangentPos;
    float3 tangentLightDir;
    float3 tangentEye;
};

[shader("vertex")] VertexOut vertex(
    uint vertexID : SV_VertexID,
    uniform GlobalData* globalData
) {
    VertexOut output;

    Vertex input = vertices[vertexID];

    output.uv = input.uv;

    float4 worldPosition  = mul(float4(input.position, 1.0), objectData.model);

    float3 worldNormal    = normalize(mul(float4(input.normal, 1.0), objectData.normal).xyz);
    float3 worldTangent   = normalize(mul(float4(input.tangent, 1.0), mul(objectData.model, globalData->view)).xyz);

    worldTangent = normalize(worldTangent - dot(worldTangent, worldNormal) * worldNormal);
    float3 worldBitangent = cross(worldNormal, worldTangent);

    output.normal = worldNormal;
    output.viewDir = normalize(globalData->eye.xyz - worldPosition.xyz);

    float3x3 TBN = transpose(float3x3(
        worldTangent,
        worldBitangent,
        worldNormal
    ));

    output.tangentPos      = mul(worldPosition.xyz, TBN);
    output.tangentLightDir = mul(globalData->lightDir.xyz, TBN);
    output.tangentEye      = mul(globalData->eye.xyz, TBN);

    output.pos = mul(worldPosition, globalData->viewProj);

    return output;
}

[shader("pixel")] PixelOut pixel(
    VertexOut input,
    uniform GlobalData* globalData
)
{
    PixelOut output;

    const float4 albedo = SampleAlbedo(input.uv);
    if (albedo.a < 0.01)
    {
        discard;
    }

    const float3 orm = SampleORM(input.uv);
    const float occlusion = orm.r;
    const float roughness = orm.g;
    const float metallic = orm.b;

    const float3 normalMap = SampleNormal(input.uv);
    const float3 normalDir = normalize(normalMap * 2.0 - 1.0);

    const float3 lightDir = input.tangentLightDir;
    const float3 viewDir = normalize(input.tangentEye - input.tangentPos);
    const float3 halfDir = normalize(viewDir + lightDir);

    float3 color;
    if (SKY_ENABLED)
    {
        color = BRDF_IBL(
            globalData,
            input.normal,
            normalDir,
            input.viewDir,
            viewDir,
            albedo.rgb,
            roughness,
            metallic,
            occlusion
        );
    }
    else
    {
        color = albedo.rgb;
    }

    const float3 emissive = SampleEmissive(input.uv);
    color += emissive;

    output.color.rgb = color;
    output.color.a = albedo.a;

    return output;
}
