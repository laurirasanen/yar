#include "common.slang"

[shader("vertex")]
VertexOutUnlit vertex(
    VertexUnlit         input,
    uniform GlobalData* globalData,
) {
    VertexOutUnlit output;

    output.pos   = mul(float4(input.pos, 1.0), globalData->viewProj);
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
