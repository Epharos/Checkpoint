// slangc.exe ShadowMapping.slang -target spirv -fvk-use-entrypoint-name -o ShadowMapping.spv

#include "Constants.slang"

struct CameraData
{
    float4x4 viewProjection[MAX_CASCADES];
    float cascadeSplits[MAX_CASCADES];
};

struct CascadeData
{
    uint cascadeIndex;
}

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
ConstantBuffer<CameraData> cascade;

[vk::binding(0, 1)]
StructuredBuffer<ModelData> models;

[vk::push_constant]
CascadeData cascadeData;

[shader("vertex")]
VSOutput vertexMain(VSInput input, uint instanceID: SV_InstanceID, out uint layer : SV_RenderTargetArrayIndex) : SV_Position
{
    VSOutput output;

    float3 fragPos = mul(models[instanceID].model, float4(input.position, 1.0f)).xyz;
    float4 clipPos = mul(cascade.viewProjection[cascadeData.cascadeIndex], float4(fragPos, 1.0f));
    clipPos.y = -clipPos.y;

    output.position = clipPos;
    layer = cascadeData.cascadeIndex;

    return output;
}