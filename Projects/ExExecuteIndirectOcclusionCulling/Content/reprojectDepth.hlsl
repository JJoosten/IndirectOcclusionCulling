// INFO: ABOVE THIS LINE, ADDITIONAL DEFINES ARE PLACED

#define DEPTH_BIAS 0.00001

// NOTE: r_ in r_cbr signals the shader reflector to add the cbv as a root parameter
cbuffer r_cbvViewInfo: register(b0)
{
	struct viewStateStruct
	{
		float4x4 ProjectionMat;
		float4x4 ViewMat;
		float2 ScreenSize;
	} viewInfo;
};

// NOTE: r_ in r_cbr signals the shader reflector to add the cbv as a root parameter
cbuffer r_cbvPrevViewInfo: register(b1)
{
	struct prevViewStateStruct
	{
		float4x4 InverseViewProj;
	} prevViewInfo;
};

SamplerState r_point_clamp : register(s3);

Texture2D<float>	prevDepthBuffer		: register(t2);	// SRV
RWTexture2D<uint>	reprojDepthBuffer	: register(u3); // UAV

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void CSMain(uint3 pixelID : SV_DispatchThreadID)
{
	float2 halfRes = viewInfo.ScreenSize / 2;

	if (pixelID.x < halfRes.x &&
		pixelID.y < halfRes.y)
	{
		float2 screenSpaceUV = float2(pixelID.xy + 1) / float2(halfRes.x, halfRes.y);

		float4 depthGather = prevDepthBuffer.GatherRed(r_point_clamp, screenSpaceUV, uint2(0, 0));

		// when we only have samples of the near plane distance, we can ignore this patch
		float depthGatherSum = depthGather.x + depthGather.y + depthGather.z + depthGather.w;
		if (depthGatherSum == 0.0)
			return;

		// we always take the maximum value of the previous depth buffer to make sure that we always have the worst case depth and dont occlude visible geometry
		float prevFrameDepth = max(depthGather.x, max(depthGather.y, max(depthGather.z, depthGather.w)));
		
		// unproject
		float4 screenSpaceToNDC = float4((screenSpaceUV * 2.0) - 1.0, prevFrameDepth, 1.0);
		screenSpaceToNDC.y = 1.0 - screenSpaceToNDC.y;
		float4 prevFrameViewSpacePos = mul(prevViewInfo.InverseViewProj, screenSpaceToNDC);
		prevFrameViewSpacePos /= prevFrameViewSpacePos.w;

		// reproject
		float4 currentFrameClipSpace = mul(viewInfo.ProjectionMat, mul(viewInfo.ViewMat, prevFrameViewSpacePos));
		float3 currentFrameNDC = currentFrameClipSpace.xyz / currentFrameClipSpace.w;
		currentFrameNDC.y = 1.0 - currentFrameNDC.y;
		float2 currentFrameScreenPos = saturate((currentFrameNDC.xy + 1.0) * 0.5) * float2(halfRes.x, halfRes.y);

		int2 currentFrameReprojectScreenPos = floor(currentFrameScreenPos);
		InterlockedMax(reprojDepthBuffer[currentFrameReprojectScreenPos], asuint(currentFrameNDC.z + DEPTH_BIAS));
	}
}
