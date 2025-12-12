// slangc.exe TexturedNormalLighting.slang -target spirv -fvk-use-entrypoint-name -o TNL.spv

#include "Constants.slang"

struct CameraData
{
    float4x4 viewProjection;
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

    output.position = float4(input.position, 1.0f);

    float4x4 mvp = mul(camera.viewProjection, models[instanceID].model);
    output.position = mul(mvp, float4(input.position, 1.0f));

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