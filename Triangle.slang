struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    VertexOutput output;

    float2 vertices[3] = { float2(0.0, 0.5), float2(0.5, -0.5), float2(-0.5, -0.5) };
    output.position = float4(vertices[vertexId], 0.0, 1.0);
    output.color = float4(1.0, 0.0, 0.0, 1.0);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_TARGET
{
    return input.color;
}