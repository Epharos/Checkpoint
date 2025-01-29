// slangc.exe TexturedNormalLighting.slang -target spirv -fvk-use-entrypoint-name -o TNL.spv

struct CameraData
{
	float4x4 viewProjection;
};

struct ModelData
{
    float4x4 model;
    float4x4 normalMatrix;
};

struct SunLight
{
    float4x4 viewProjection;
    float4 lightDirection;
    float4 lightColor;
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
    float4 lightSpacePosition : LIGHTSPACEPOSITION;
    float3 lightDirection;
    float3 lightColor;
    float3 viewPosition;
};

[vk::binding(0)]
ConstantBuffer<CameraData> camera;

[vk::binding(1, 0)]
ConstantBuffer<SunLight> lightData;

[vk::binding(2, 0)]
Texture2D shadowMapTexture;

[vk::binding(3, 0)]
SamplerComparisonState shadowMapSampler;

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
    output.lightSpacePosition.y = -output.lightSpacePosition.y;

    output.lightDirection = normalize(lightData.lightDirection.xyz);
    output.lightColor = lightData.lightColor.xyz;

    output.viewPosition = camera.viewProjection[3].xyz;

	return output;
}

float ComputeShadow(float4 lightSpacePosition, float3 lightDirection, float3 fragNormal)
{
    // Perform the perspective divide
    float2 projCoords = lightSpacePosition.xy / lightSpacePosition.w;
    float currentDepth = lightSpacePosition.z / lightSpacePosition.w;

    // Transform the [-1, 1] range to [0, 1] for x and y (not z)
    projCoords = projCoords * 0.5 + 0.5;

    // If the point is outside the light's frustum, it is lighten
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
	{
		return 1.0;
	}

    //return currentDepth;
    //return currentDepth - bias > shadowMapTexture.Sample(shadowMapSampler, projCoords.xy).r ? 0.0 : 1.0;

    // And we return a value in {0, 1} where 0 is in shadow and 1 is not
    //return shadowMapTexture.SampleCmp(shadowMapSampler, projCoords.xy, currentDepth) > 0.0 ? 1.0 : 0.0;
    return PercentageCloserFiltering(projCoords.xy, currentDepth, 2);
}

float3 ComputeShadowDebug(float4 lightSpacePosition, float3 lightDirection, float3 fragNormal)
{
    	// Perform the perspective divide
	float2 projCoords = lightSpacePosition.xy / lightSpacePosition.w;
	float currentDepth = lightSpacePosition.z / lightSpacePosition.w;

	// Transform the [-1, 1] range to [0, 1] for x and y (not z)
	projCoords = projCoords * 0.5 + 0.5;

	// If the point is outside the light's frustum, it is lighten
	if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
	{
		return float3(1.0, 0.0, 0.0);
	}

    if(shadowMapTexture.SampleCmp(shadowMapSampler, projCoords.xy, currentDepth) > 0.0)
	{
		return float3(0.0, 1.0, 0.0);
	}

	return float3(0.0, 0.0, 1.0);

}

float PercentageCloserFiltering(float2 uv, float currentDepth, int size)
{
	float shadow = 0.0;
    float texelSize = 1.0 / 4096.0;

	for(int x = -size; x <= size; x++)
	{
		for(int y = -size; y <= size; y++)
		{
            float2 offset = float2(x, y) * texelSize;
            shadow += shadowMapTexture.SampleCmp(shadowMapSampler, uv + offset, currentDepth) > 0.0 ? 1.0 : 0.0;
		}
	}

	return shadow / ((size + 1) * (size + 1));
}

[shader("pixel")]
float4 pixelMain(VSOutput input) : SV_TARGET
{
    // tmp
    float3 albedo = albedo.Sample(input.uv * textureData.scale).xyz; // We sample the albedo to get the color of the pixel

    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal); // Create the TBN matrix
    float3 normalMapValue = normal.Sample(input.uv * textureData.scale).xyz * 2.0 - 1.0; // Sample the normal map and convert it from [0, 1] to [-1, 1]
    float3 normalValue = normalize(mul(TBN, normalMapValue)); // Turns the normal map value into a normal in tangent space

    float3 ambiantColor = 0.08 * input.lightColor;
    float3 diffuse = max(dot(normalValue, -input.lightDirection), 0.0);

    //float3 finalColor = albedo * saturate((ambiantColor + (ComputeShadow(input.lightSpacePosition, input.lightDirection, normalValue) * diffuse))); // Compute the final color
    float3 finalColor = albedo * (saturate(ambiantColor + ComputeShadow(input.lightSpacePosition, input.lightDirection, normalValue)));
    return float4(finalColor, 1.0);
    // return float4(ComputeShadowDebug(input.lightSpacePosition, input.lightDirection, normalValue), 1.0);
    //return float4(0.0, input.normal.y, 0.0, 1.0);
}

float3x3 f3x3(float4x4 m) {
	return float3x3(m[0].xyz, m[1].xyz, m[2].xyz);
}