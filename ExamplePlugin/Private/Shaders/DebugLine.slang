struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
}

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
}

struct CameraData
{
    float4x4 viewProjection;
}

[[vk::binding(0, 0)]]
ConstantBuffer<CameraData> camera;

[shader("vertex")]
VertexOutput VSMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(camera.viewProjection, float4(input.position, 1.0));
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 FSMain(VertexOutput input) : SV_Target
{
    return input.color;
}
