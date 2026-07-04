#ifndef COMMON_SLANG
#define COMMON_SLANG

#define PI 3.14159265358979323846

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

struct PixelOut
{
    float4 color : SV_Target0;
};

#endif // COMMON_SLANG
