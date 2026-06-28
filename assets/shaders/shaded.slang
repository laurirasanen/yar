#include "common.slang"
#include "color.slang"
#include "texture.slang"
#include "brdf.slang"

[shader("vertex")] VertexOutShaded vertex(
    VertexShaded        input,
    uniform GlobalData* globalData,
) {
    VertexOutShaded output;

    output.uv = input.uv;

    float4 worldPosition  = mul(float4(input.pos, 1.0), objectData.model);

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
    output.tbn = TBN;

    output.pos = mul(worldPosition, globalData->viewProj);

    return output;
}

[shader("pixel")] PixelOut pixel(
    VertexOutShaded     input,
    uniform GlobalData* globalData,
)
{
    PixelOut output;

    float4 albedo = texAlbedo[objectData.index].Sample(input.uv);
    if (albedo.a < 0.01)
    {
        discard;
    }

    float3 lightDir = input.tangentLightDir;
    float3 viewDir = normalize(input.tangentEye - input.tangentPos);
    float3 halfDir = normalize(viewDir + lightDir);

    float3 normalMap = texNormal[objectData.index].Sample(input.uv).rgb;
    float3 normalDir = normalize(normalMap * 2.0 - 1.0);

    float3 orm = texMRAO[objectData.index].Sample(input.uv).rgb;
    float occlusion = pow(orm.r, objectData.materialParams.x);
    float roughness = orm.g * objectData.materialParams.y;
    float metalness = orm.b * objectData.materialParams.z;

    float3 color = BRDF_IBL(
        globalData,
        input.normal,
        normalDir,
        input.viewDir,
        viewDir,
        albedo.rgb,
        roughness,
        metalness,
        occlusion
    );

    /* Emissive */
    const float4 emissiveStrength = float4(objectData.materialParams2.xyz, 1.0);
    const float4 emissiveTex = texEmissive[objectData.index].Sample(input.uv).rgba;
    const float4 emissive = emissiveTex * emissiveStrength;
    color += emissive.rgb * emissive.a;

    /* Post-processing */
    const float exposure = globalData->settings[0];
    color = color * exposure;
    color = ACESTonemap(color);
    const float contrast = globalData->settings[1];
    color = pow(color, contrast);

    output.color.rgb = color;
    output.color.a = albedo.a;

    return output;
}
