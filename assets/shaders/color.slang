float luminance(float3 color)
{
    return dot(color, float3(0.299, 0.587, 0.114));
}

// https://www.itu.int/rec/R-REC-BT.709 (BT.709-6 06/2015)
float _linear_to_bt709(float L)
{
    const float low = 4.5 * L;
    const float high = 1.099 * pow(L, 0.45) - 0.099;
    const float mult = step(L, 0.018);
    return (1.0 - mult) * low + mult * high;
}

float _bt709_to_linear(float x)
{
    const float low = x / 4.5;
    const float high = pow((x + 0.099) / 1.099, 1.0 / 0.45);
    const float mult = step(0.081, x);
    return (1.0 - mult) * low + mult * high;
}

float3 linear_to_bt709(float3 color)
{
    return float3(
        _linear_to_bt709(color.r),
        _linear_to_bt709(color.g),
        _linear_to_bt709(color.b)
    );
}

float3 bt709_to_linear(float3 color)
{
    return float3(
        _bt709_to_linear(color.r),
        _bt709_to_linear(color.g),
        _bt709_to_linear(color.b)
    );
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 ACESTonemap(float3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0, 1);
}
