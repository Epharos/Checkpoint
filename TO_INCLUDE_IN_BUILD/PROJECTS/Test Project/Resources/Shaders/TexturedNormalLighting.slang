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

struct SunLight
{
    float4 lightDirection;
    float4 lightColor;
    uint cascadeCount;
}

struct Cascades
{
    float4x4 viewProjection[MAX_CASCADES];
    float4 cascadeSplits[MAX_CASCADES / 4];
}

struct TextureData
{
    float scale;
};

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

[vk::binding(1, 0)]
ConstantBuffer<SunLight> lightData;

[vk::binding(2, 0)]
ConstantBuffer<Cascades> cascadeData;

[vk::binding(3, 0)]
Texture2DArray shadowMapTextures;

[vk::binding(4, 0)]
SamplerComparisonState shadowMapSampler;

[vk::binding(0, 1)]
StructuredBuffer<ModelData> models;

[vk::binding(0, 2)]
Sampler2D albedo;

[vk::binding(1, 2)]
Sampler2D normal;

[vk::binding(2, 2)]
ConstantBuffer<TextureData> textureData;

float GetVectorComponentFromIndex(float4 vector, uint index)
{
	switch(index)
	{
		case 0:
			return vector.x;
		case 1:
			return vector.y;
		case 2:
			return vector.z;
		case 3:
			return vector.w;
		default:
			return 0.0;
	}
}

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

    // Calculate the light spaces position
    for (uint i = 0; i < lightData.cascadeCount; i++)
    {
        output.lightSpacePosition[i] = mul(cascadeData.viewProjection[i], worldPos);
        output.lightSpacePosition[i].y = -output.lightSpacePosition[i].y;
        output.cascadeSplits[i] = GetVectorComponentFromIndex(cascadeData.cascadeSplits[i / 4], i % 4);
	}

    output.lightDirection = normalize(lightData.lightDirection.xyz);
    output.lightColor = lightData.lightColor;

    output.viewPosition = camera.viewProjection[3].xyz;

    output.nearFar = camera.nearFar;

	return output;
}

float ComputeShadow(float4 lightSpacePosition[MAX_CASCADES], float3 lightDirection, float3 fragNormal, uint cascadeIndex)
{
    if(cascadeIndex == -1)
    {
        return 1.0;
	}

    float2 projCoords = lightSpacePosition[cascadeIndex].xy / lightSpacePosition[cascadeIndex].w;
    float currentDepth = lightSpacePosition[cascadeIndex].z / lightSpacePosition[cascadeIndex].w;

    // Transform the [-1, 1] range to [0, 1] for x and y (not z)
    projCoords = projCoords * 0.5 + 0.5;

    // If the point is outside the light's frustum, it is lighten
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
	{
		return 1.0;
	}

    //return shadowMapTextures.SampleCmp(shadowMapSampler, float3(projCoords.xy, cascadeIndex), currentDepth) > 0.0 ? 1.0 : 0.0;

    return PercentageCloserFiltering(float3(projCoords.xy, float(cascadeIndex)), currentDepth, 2);
}

float PercentageCloserFiltering(float3 uvl, float currentDepth, int size)
{
    float shadow = 0.0;
    uint width, height, elements;
    shadowMapTextures.GetDimensions(width, height, elements);
    float texelSize = 1.0 / float(width);

	for(int x = -size; x <= size; x++)
	{
		for(int y = -size; y <= size; y++)
		{
            float2 offset = float2(x, y) * texelSize;
            float3 samplePos = float3(uvl.xy + offset, uvl.z);
            shadow += shadowMapTextures.SampleCmp(shadowMapSampler, samplePos, currentDepth) > 0.0 ? 1.0 : 0.0;
		}
	}

	return shadow / ((size + 1) * (size + 1));
}

float3 DebugCascade(uint cascadeIndex)
{
    if(cascadeIndex == -1)
    {
		return float3(0.0, 0.0, 0.0);
	}

    if(cascadeIndex == 0)
        return float3(1.0, 0.0, 0.0);
    else if(cascadeIndex == 1)
        return float3(0.0, 1.0, 0.0);
	else if(cascadeIndex == 2)
		return float3(0.0, 0.0, 1.0);
	else if(cascadeIndex == 3)
		return float3(1.0, 1.0, 0.0);
	else if(cascadeIndex == 4)
		return float3(1.0, 0.0, 1.0);
	else if(cascadeIndex == 5)
		return float3(0.0, 1.0, 1.0);
	else if(cascadeIndex == 6)
		return float3(1.0, 1.0, 1.0);
	else if(cascadeIndex == 7)
		return float3(0.5, 0.5, 0.5);

	return float3(0.0, 0.0, 0.0);
}


[shader("pixel")]
float4 pixelMain(VSOutput input) : SV_TARGET
{
    // tmp
    float3 albedo = albedo.Sample(input.uv * textureData.scale).xyz; // We sample the albedo to get the color of the pixel

    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal); // Create the TBN matrix
    float3 normalMapValue = normal.Sample(input.uv * textureData.scale).xyz * 2.0 - 1.0; // Sample the normal map and convert it from [0, 1] to [-1, 1]
    float3 normalValue = normalize(mul(TBN, normalMapValue)); // Turns the normal map value into a normal in tangent space

    float3 ambiantColor = 0.08 * input.lightColor.rgb;
    float3 diffuse = max(dot(normalValue, -input.lightDirection), 0.0);

    float depth = (input.eyeSpacePosition.z - input.nearFar.x) / (input.nearFar.y - input.nearFar.x);

    uint cascadeIndex = -1;

    for (uint i = 0; i < lightData.cascadeCount; i++)
    {
        if (input.eyeSpacePosition.z < input.cascadeSplits[i])
        {
            cascadeIndex = i;
            break;
        }
    }

    float3 finalColor = albedo * (saturate(ambiantColor + ComputeShadow(input.lightSpacePosition, input.lightDirection, normalValue, cascadeIndex)));

    // return float4(albedo * ComputeShadow(input.lightSpacePosition, input.cascadeSplits, depth, input.lightDirection, normalValue), 1.0);
    return float4(finalColor, 1.0);
}

float3x3 f3x3(float4x4 m) {
	return float3x3(m[0].xyz, m[1].xyz, m[2].xyz);
}