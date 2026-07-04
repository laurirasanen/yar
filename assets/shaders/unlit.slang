#include "bindings.slang"
#include "common.slang"

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

[shader("vertex")]
VertexOutUnlit vertex(
    VertexUnlit         input,
    uniform GlobalData* globalData,
) {
    VertexOutUnlit output;

    output.pos   = mul(float4(input.pos, 1.0), MAT_MVP);
    output.color = input.color;

    return output;
}

[shader("pixel")]
PixelOut pixel(
    VertexOutUnlit input,
    uniform GlobalData* globalData,
) {
    PixelOut output;

    output.color = float4(input.color, 1.0);

    return output;
}
