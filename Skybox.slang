struct SkyboxUBO
{
    float4x4 invViewRotationProj;
}

struct VertexOutput
{
    float4 position : SV_Position;
    float3 viewDir  : TEXCOORD0;
}

[[vk_binding(0, 0)]]
ConstantBuffer<SkyboxUBO> ubo;

[[vk_binding(1, 0)]]
SamplerState cubeSampler;

[[vk_binding(2, 0)]]
TextureCube skyboxTexture;

[shader("vertex")]
VertexOutput VSMain(uint vertexId : SV_VertexID)
{
    float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
    float2 ndc = uv * 2.0f - 1.0f;

    float4 worldDir4 = mul(ubo.invViewRotationProj, float4(ndc, 1.0f, 1.0f));
    worldDir4 /= worldDir4.w;

    VertexOutput output;
    output.position = float4(ndc, 1.0f, 1.0f);
    output.viewDir = worldDir4.xyz;
    return output;
}

[shader("fragment")]
float4 FSMain(VertexOutput input)
{
    return skyboxTexture.Sample(cubeSampler, normalize(input.viewDir));
}
