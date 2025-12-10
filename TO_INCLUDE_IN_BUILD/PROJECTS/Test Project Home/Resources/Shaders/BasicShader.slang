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
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;

    float3 worldPosition : TEXCOORD1;
    float3 eyeSpacePosition : TEXCOORD2;

    float2 nearFar;

    float4 lightSpacePosition[MAX_CASCADES];
    float cascadeSplits[MAX_CASCADES];

    float3 lightDirection;
    float4 lightColor;

    float3 viewPosition;
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

    float4 eyeSpace = mul(camera.view, worldPos);
    output.eyeSpacePosition = eyeSpace.xyz;
    output.eyeSpacePosition.z = -output.eyeSpacePosition.z;

    // Calculate the model-view-projection matrix and multiply it with the vertex position to get the clip-space position
    output.position = mul(camera.viewProjection, worldPos);

	//The UV remains the same no matter what
	output.uv = input.uv;

    // The normal matrix was sent as a float4x4 (because of alignment) so we need to convert it to a float3x3
    float3x3 normalMatrix = f3x3(models[instanceID].normalMatrix);

    // Calculate the normal in clip-space
    output.normal = normalize(mul(normalMatrix, input.normal));

    // Calculate the tangent in clip-space
    output.tangent = normalize(mul(normalMatrix, input.tangent));

    // Calculate the bitangent in clip-space
    output.bitangent = normalize(mul(normalMatrix, input.bitangent));

	return output;
}

[shader("pixel")]
float4 pixelMain(VSOutput input) : SV_TARGET
{
    return textureData.meshColor;
}

float3x3 f3x3(float4x4 m) {
	return float3x3(m[0].xyz, m[1].xyz, m[2].xyz);
}