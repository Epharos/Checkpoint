struct VSInput
{
	float3 position : POSITION;
    float3 color : COLOR;
}

struct VSOutput
{
	float4 position : SV_POSITION;
	float3 color : COLOR;
}

[shader("vertex")]
VSOutput vs_main(VSInput input)
{
	VSOutput output;
	output.position = float4(input.position, 1.0f);
	output.color = input.color;
	return output;
}

[shader("pixel")]
float3 ps_main(VSOutput input) : SV_TARGET
{
	return input.color;
}