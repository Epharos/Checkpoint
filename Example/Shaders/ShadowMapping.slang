// slangc.exe ShadowMapping.slang -target spirv -fvk-use-entrypoint-name -o ShadowMapping.spv

struct CameraData
{
    float4x4 viewProjection;
};

struct ModelData
{
    float4x4 model;
    float4x4 normalMatrix;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
}

struct VSOutput
{
    float4 position : SV_Position;
    uint layer : SV_RenderTargetArrayIndex;
}

[vk::binding(0)]
ConstantBuffer<CameraData> camera;

[vk::binding(0, 1)]
StructuredBuffer<ModelData> models;

[shader("vertex")]
float4 vertexMain(VSInput input, uint instanceID: SV_InstanceID) : SV_Position
{
    float3 fragPos = mul(models[instanceID].model, float4(input.position, 1.0f)).xyz;
    float4 clipPos = mul(camera.viewProjection, float4(fragPos, 1.0f));
    clipPos.y = -clipPos.y;
    // Calculate the model-view-projection matrix and multiply it with the vertex position to get the clip-space position
    return clipPos;
}