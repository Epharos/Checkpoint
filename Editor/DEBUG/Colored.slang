module Colored;

public struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 color : COLOR;
};

public struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 color : COLOR;
};

[shader("vertex")]
public VSOutput VertexDefault(in VSInput input)
{
    VSOutput output;
    output.position = float4(input.position, 1.0);
    output.normal = input.normal;
    output.color = input.color;
    return output;
}

[shader("pixel")]
public float4 FragmentDefault(in VSOutput input) : SV_TARGET
{
    return float4(input.color, 1.0);
}