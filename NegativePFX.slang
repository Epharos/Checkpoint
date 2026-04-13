[vk::binding(0, 0)]
RWTexture2D colorTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
void MainCS(uint3 threadId : SV_DispatchThreadID)
{
    float4 color = colorTexture.Load(int2(threadId.x, threadId.y));
    color.xyz = float3(1, 1, 1) - color.xyz;
    colorTexture.Store(int2(threadId.x, threadId.y), color);
}