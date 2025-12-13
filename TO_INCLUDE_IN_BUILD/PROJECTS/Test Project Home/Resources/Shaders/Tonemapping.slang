struct VSInput
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

[vk::binding(0, 2)]
Sampler2D screenColorTexture;

[shader("vertex")]
VSOutput VS_Main(VSInput input)
{
    VSOutput output;
    output.position = float4(input.position, 0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}

[shader("pixel")]
float4 PS_Main(VSOutput input) : SV_TARGET
{
    float4 color = screenColorTexture.Sample(input.uv);
    return float4(ACESFilm(color.rgb), 1.0);
}

float3 ACESFilm(float3 color)
{
    let a = 2.51;
    let b = 0.03;
    let c = 2.43;
    let d = 0.59;
    let e = 0.14;
    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

float3 HableTonemap(float3 color)
{
    color *= 16.0; // Exposure adjustment
    let A = 0.15;
    let B = 0.50;
    let C = 0.10;
    let D = 0.20;
    let E = 0.02;
    let F = 0.30;

    return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - (E / F);
}

float3 UnrealTonemap(float3 color)
{
    return color / (color + 0.155f) * 1.019f;
}