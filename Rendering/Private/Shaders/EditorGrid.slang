struct CameraData
{
    float4x4 viewProjection;
    float4x4 inverseViewProjection;
    float4 worldPositionAndNear;  // xyz = world position, w = near
    float4 farAndPadding; // x = far
}

struct GridParams
{
    float4 color;
    float cellSize;
    float lineThickness;
    float fadeStartDistance;
    float fadeEndDistance;
    uint ignoreDepth;
    float3 padding;
}

[[vk::binding(0, 0)]]
RWTexture2D<float4> colorTexture;

[[vk::binding(1, 0)]]
Texture2D<float> depthTexture;

[[vk::binding(2, 0)]]
ConstantBuffer<CameraData> camera;

[[vk::binding(3, 0)]]
ConstantBuffer<GridParams> gridParams;

float LinearDepth(float d)
{
    const float n = camera.worldPositionAndNear.w;
    const float f = camera.farAndPadding.x;
    return (n * f) / (f - d * (f - n));
}

bool RayGroundPlane(float2 ndc, out float3 worldPos, out float gridNdcDepth)
{
    float4 nearH = mul(camera.inverseViewProjection, float4(ndc, 0.0, 1.0));
    float3 nearW = nearH.xyz / nearH.w;
    float4 farH = mul(camera.inverseViewProjection, float4(ndc, 1.0, 1.0));
    float3 farW = farH.xyz / farH.w;

    float3 dir = farW - nearW;
    worldPos = float3(0.0, 0.0, 0.0);
    gridNdcDepth = 0.0;

    if (abs(dir.y) < 1e-5)
    {
        return false;
    }

    float t = -nearW.y / dir.y;
    if (t < 0.0)
    {
        return false;
    }

    worldPos = nearW + t * dir;

    float4 proj  = mul(camera.viewProjection, float4(worldPos, 1.0));
    gridNdcDepth = proj.z / proj.w;

    if (gridNdcDepth < 0.0 || gridNdcDepth > 1.0)
    {
        return false;
    }

    return true;
}

float2 PixelToNDC(float2 pixel, float2 invSize)
{
    return pixel * invSize * 2.0 - 1.0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void MainCS(uint3 threadId : SV_DispatchThreadID)
{
    uint width, height;
    colorTexture.GetDimensions(width, height);
    if (threadId.x >= width || threadId.y >= height)
    {
        return;
    }

    float2 invSize = 1.0 / float2(width, height);
    float2 center = float2(threadId.xy) + 0.5;

    float3 worldPos;
    float gridNdcDepth;

    if (!RayGroundPlane(PixelToNDC(center, invSize), worldPos, gridNdcDepth))
    {
        return;
    }

    if (gridParams.ignoreDepth == 0)
    {
        float sceneDepth = depthTexture.Load(int3(threadId.xy, 0));
        float linearScene = LinearDepth(sceneDepth);
        float linearGrid = LinearDepth(gridNdcDepth);

        if (linearGrid > linearScene + 0.01)
        {
            return;
        }
    }

    float3 worldPosX, worldPosY;
    float dummyDepth;
    bool hitX = RayGroundPlane(PixelToNDC(center + float2(1.0, 0.0), invSize), worldPosX, dummyDepth);
    bool hitY = RayGroundPlane(PixelToNDC(center + float2(0.0, 1.0), invSize), worldPosY, dummyDepth);

    float2 fw_xz;
    if (hitX && hitY)
    {
        fw_xz = float2(
            max(abs(worldPosX.x - worldPos.x), abs(worldPosY.x - worldPos.x)),
            max(abs(worldPosX.z - worldPos.z), abs(worldPosY.z - worldPos.z))
        );
    }
    else
    {
        fw_xz = float2(gridParams.cellSize * 0.02, gridParams.cellSize * 0.02);
    }

    float2 gridP = worldPos.xz / gridParams.cellSize;
    float2 f = frac(gridP);
    float2 dist = min(f, 1.0 - f); // distance to nearest line in [0, 0.5]

    float2 fw = max(fw_xz / gridParams.cellSize, 0.001);
    float2 coverage = 1.0 - smoothstep(
        gridParams.lineThickness - fw,
        gridParams.lineThickness + fw,
        dist
    );

    float grid = max(coverage.x, coverage.y);

    if (grid <= 0.0)
    {
        return;
    }

    float3 camPos = camera.worldPositionAndNear.xyz;
    float distCam = length(worldPos.xz - camPos.xz);
    float fade = 1.0 - smoothstep(
        gridParams.fadeStartDistance,
        gridParams.fadeEndDistance,
        distCam
    );

    if (fade <= 0.0)
    {
        return;
    }

    float4 existing = colorTexture.Load(int2(threadId.xy));
    float alpha = gridParams.color.a * grid * fade;
    float3 blended = lerp(existing.rgb, gridParams.color.rgb, alpha);
    colorTexture.Store(int2(threadId.xy), float4(blended, existing.a));
}
