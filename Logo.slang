struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

[vk::binding(0, 0)]
Sampler2D logo;

[shader("vertex")]
VertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    VertexOutput output;

    float2 vertices[6] = {
        float2(-0.5, 0.5), float2(0.5, -0.5), float2(-0.5, -0.5),
        float2(-0.5, 0.5), float2(0.5, 0.5), float2(0.5, -0.5)
    };

    float2 uvs[6] = {
        float2(0, 1), float2(1, 0), float2(0, 0),
        float2(0, 1), float2(1, 1), float2(1, 0)
    };

    output.position = float4(vertices[vertexId], 0.0, 1.0);
    output.uv = uvs[vertexId];
    output.color = float4(1.0, 0.0, 0.0, 1.0);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_TARGET
{
    float3 logoColor = logo.Sample(input.uv).xyz;
    return float4(logoColor, 1.0);
}