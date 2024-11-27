[[vk::binding(0)]]
struct CameraData
{
	float4x4 view;
	float4x4 projection;
} camera;

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
	float4x4 viewProj = mul(camera.projection, camera.view);
	output.position = mul(viewProj, float4(input.position, 1.0f));
	output.color = input.color;
	return output;
}

[shader("pixel")]
float3 ps_main(VSOutput input) : SV_TARGET
{
	return input.color;
}