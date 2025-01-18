// slangc.exe ColorNormalLighting.slang -target spirv -fvk-use-entrypoint-name -o CNL.spv

struct CameraData
{
	float4x4 view;
	float4x4 projection;
};

struct ModelData
{
    float4x4 model;
    float4x4 normalMatrix;
}

struct ColorMaterial
{
    float3 color;
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
	float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
}

[vk::binding(0)]
ConstantBuffer<CameraData> camera;

[vk::binding(0, 1)]
StructuredBuffer<ModelData> models;

[vk::binding(0, 2)]
ConstantBuffer<ColorMaterial> material;

[shader("vertex")]
VSOutput vertexMain(VSInput input, uint instanceID : SV_InstanceID)
{
	VSOutput output;

	//Calculate the model-view-projection matrix and multiply it with the vertex position to get the clip-space position
	float4x4 viewProj = mul(camera.projection, camera.view);
	float4x4 mvp = mul(viewProj, models[instanceID].model);
	output.position = mul(mvp, float4(input.position, 1.0f));

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
    // tmp
    float3 color = material.color; // Stores the color of the material
    float3 sunDir = normalize(float3(0.0, -1.0, 0.0)); // This is just a placeholder direction while we don't have a light DS

    float3 normalValue = normalize(input.normal + 1.0) * 0.5; // Convert the normal from [-1, 1] to [0, 1] > used for debugging

    float3 ambiantColor = float3(0.03, 0.03, 0.08); // This is just a placeholder color while we don't have a light DS
    float3 diffuse = max(dot(input.normal, sunDir), 0.0); // This is just a placeholder color while we don't have a light DS

    float3 finalColor = color * (diffuse + ambiantColor); // Compute the final color
    return float4(finalColor, 1.0);
}

float3x3 f3x3(float4x4 m) {
	return float3x3(m[0].xyz, m[1].xyz, m[2].xyz);
}