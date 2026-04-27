struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 color : COLOR;
}

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 color : COLOR;
}

struct CameraData
{
    float4x4 viewProjection;
}

struct InstanceData
{
    float4x4 model;
}

[[vk_binding(0, 0)]]
ConstantBuffer<CameraData> camera;

[[vk_binding(1, 0)]]
StructuredBuffer<InstanceData> instances;

[shader("vertex")]
VertexOutput VSMain(VertexInput input, uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    VertexOutput output;

    float4x4 mvp = mul(camera.viewProjection, instances[instanceId].model);
    float4 positionMS = mul(instances[instanceId].model, float4(input.position, 1));

    output.position = mul(mvp, float4(input.position, 1));
    output.normal = normalize(mul(mvp, float4(input.normal, 0)).xyz);
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 FSMain(VertexOutput input)
{
    const float3 ambiantColor = float3(0.1, 0.1, 0.1);
    const float3 lightDirection = normalize(float3(0.33, -0.5, 0.25));

    const float NdotH = saturate(dot(input.normal, lightDirection));

    return float4(ambiantColor, 1) + NdotH * float4(1, 1, 0, 1);
}