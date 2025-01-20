// slangc.exe TexturedNormalLighting.slang -target spirv -fvk-use-entrypoint-name -o TNL.spv

struct CameraData
{
	float4x4 viewProjection;
};

struct ModelData
{
    float4x4 model;
    float4x4 normalMatrix;
}

struct TextureData
{
    float scale;
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
    float4 lightSpacePosition : LIGHTSPACEPOSITION;
}

[vk::binding(0)]
ConstantBuffer<CameraData> camera;

[vk::binding(1, 0)]
ConstantBuffer<CameraData> lightData; // For now we are using CameraData struct but we should create a LightData struct

[vk::binding(2, 0)]
Texture2D shadowMapTexture;

[vk::binding(3, 0)]
SamplerState shadowMapSampler;

[vk::binding(0, 1)]
StructuredBuffer<ModelData> models;

[vk::binding(0, 2)]
Sampler2D albedo;

[vk::binding(1, 2)]
Sampler2D normal;

[vk::binding(2, 2)]
ConstantBuffer<TextureData> textureData;

[shader("vertex")]
VSOutput vertexMain(VSInput input, uint instanceID : SV_InstanceID)
{
	VSOutput output;

    float3 fragPos = mul(models[instanceID].model, float4(input.position, 1.0f)).xyz;

	//Calculate the model-view-projection matrix and multiply it with the vertex position to get the clip-space position
	output.position = mul(camera.viewProjection, float4(fragPos, 1.0));

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

    // Calculate the light space position
    output.lightSpacePosition = mul(lightData.viewProjection, float4(fragPos, 1.0f));

	return output;
}

float LinearizeDepth(float depth, float near, float far)
{
    return (2.0 * near * far) / (far + near - depth * (far - near));
}

float ComputeShadow(float4 lightSpacePosition)
{
    // Perform the perspective divide
    float3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;

    // Transform the [-1, 1] range to [0, 1]
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z < 0.0 || projCoords.z > 1.0)
	{
		return 1.0;
	}

    float bias = 0.08;
    float currentDepth = projCoords.z;
    //float depthMapValue = LinearizeDepth(shadowMapTexture.Sample(shadowMapSampler, projCoords.xy).r, 0.1, 1000.0);
    float depthMapValue = shadowMapTexture.Sample(shadowMapSampler, projCoords.xy).r;
    //return float3(projCoords.xy, 0);
    //return /*cos(projCoords.x * 3.14) > 0 ? currentDepth : */currentDepth;
    return currentDepth > (depthMapValue + bias) ? 1.0 : 0.0;

    //return shadowMapTexture.SampleCmp(shadowMapSampler, projCoords.xy, projCoords.z - bias);
}

[shader("pixel")]
float4 pixelMain(VSOutput input) : SV_TARGET
{
    // tmp
    float3 albedo = albedo.Sample(input.uv * textureData.scale).xyz; // We sample the albedo to get the color of the pixel
    float3 sunDir = normalize(float3(0.0, 1.0, 0.0)); // This is just a placeholder direction while we don't have a light DS

    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal); // Create the TBN matrix
    float3 normalMapValue = normal.Sample(input.uv * textureData.scale).xyz * 2.0 - 1.0; // Sample the normal map and convert it from [0, 1] to [-1, 1]
    float3 normalValue = normalize(mul(TBN, normalMapValue)); // Turns the normal map value into a normal in tangent space

    float3 ambiantColor = float3(0.01, 0.01, 0.01); // This is just a placeholder color while we don't have a light DS
    float3 diffuse = max(dot(normalValue, sunDir), 0.0);

    float3 finalColor = albedo * (ambiantColor + ComputeShadow(input.lightSpacePosition) * diffuse); // Compute the final color
    //return float4(float3(ComputeShadow(input.lightSpacePosition)), 1.0);
    return float4(finalColor, 1.0);
}

float3x3 f3x3(float4x4 m) {
	return float3x3(m[0].xyz, m[1].xyz, m[2].xyz);
}