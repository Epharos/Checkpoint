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
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;

    uint instanceID;
};

[vk::binding(0)]
ConstantBuffer<CameraData> camera;

[vk::binding(0, 1)]
StructuredBuffer<ModelData> models;

[vk::binding(0, 2)]
ConstantBuffer<MeshData> textureData;

[shader("vertex")]
VSOutput vertexMain(VSInput input, uint instanceID : SV_InstanceID, uint baseInstanceID : SV_StartInstanceLocation)
{
	VSOutput output;

    uint instanceIndex = instanceID + baseInstanceID;

    output.position = float4(input.position, 1.0f);

    float4x4 mvp = mul(camera.viewProjection, models[instanceIndex].model);
    output.position = mul(mvp, float4(input.position, 1.0f));

    output.normal = input.normal;
    output.instanceID = instanceIndex;

	return output;
}

[shader("pixel")]
float4 pixelMain(VSOutput input) : SV_TARGET
{
    float3 lightDir = normalize(float3(1.0f, 1.0f, -1.0f));
    float NdotL = max(dot(input.normal, lightDir), 0.0f);
    return float4(textureData.meshColor.rgb * NdotL, 1.0);
}