// slangc.exe HALF_QUAD.slang -target spirv -fvk-use-entrypoint-name -o HALF_QUAD.spv

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

[shader("vertex")]
VSOutput vertexMain(VSInput input, uint instanceID : SV_InstanceID)
{
	VSOutput output;

	output.position = float4(input.position, 1.0f);

	return output;
}

[shader("pixel")]
float4 pixelMain(VSOutput input) : SV_TARGET
{
    return float4(1, 1, 0, 1);
}