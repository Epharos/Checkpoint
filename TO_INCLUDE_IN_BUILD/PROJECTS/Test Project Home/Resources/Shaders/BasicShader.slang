// slangc.exe TexturedNormalLighting.slang -target spirv -fvk-use-entrypoint-name -o TNL.spv

#include "Constants.slang"

struct CameraData
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
	float2 nearFar;
};

struct ModelData
{
    float4x4 model;
    float4x4 normalMatrix;
};

struct MeshData
{
    float4 meshColor;
}

struct VSInput
{
	float3 position : POSITION;
    float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VSOutput
{
	float4 position : SV_POSITION;
    
    float3 worldPosition : TEXCOORD1;
};

[vk::binding(0)]
ConstantBuffer<CameraData> camera;

[vk::binding(0, 1)]
StructuredBuffer<ModelData> models;

[vk::binding(0, 2)]
ConstantBuffer<MeshData> textureData;

[shader("vertex")]
VSOutput vertexMain(VSInput input, uint instanceID : SV_InstanceID)
{
	VSOutput output;

    float4 worldPos = mul(models[instanceID].model, float4(input.position, 1.0f));

    output.worldPosition = worldPos.xyz;

    // Calculate the model-view-projection matrix and multiply it with the vertex position to get the clip-space position
    output.position = mul(camera.viewProjection, worldPos);

    // Fix to avoid unused variable warnings
    float3 normal = input.normal;
    float3 tangent = input.tangent;
    float3 bitangent = input.bitangent;
    float2 uv = input.uv;

	return output;
}

[shader("pixel")]
float4 pixelMain(VSOutput input) : SV_TARGET
{
    return textureData.meshColor;
}