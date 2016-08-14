// INFO: ABOVE THIS LINE, ADDITIONAL DEFINES ARE PLACED

#define allZero(x) (!any(x))

cbuffer rc_constants : register(b3)
{
	float2 ScreenSize;
};

SamplerState r_point_clamp : register(s2);

Texture2D<float>	reprojDepthBuffer		: register(t0);	// SRV
RWTexture2D<float>	downSampleDepthBuffer	: register(u1); // UAV

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void CSMain(uint3 pixelID : SV_DispatchThreadID)
{
	float2 quarterRes = ScreenSize / 4.0;

	if (pixelID.x < quarterRes.x &&
		pixelID.y < quarterRes.y)
	{
		float2 uv = float2(pixelID.xy + 0.5) / quarterRes;

		float4 gatheredSamples = reprojDepthBuffer.GatherRed(r_point_clamp, uv, uint2(0, 0));

		if (allZero(gatheredSamples))
		{
			downSampleDepthBuffer[pixelID.xy] = 1.0;
		}
		else
		{
			// we always take the maximum value of the previous depth buffer, to make sure that we always have the worst case depth and dont occlude visible geometry
			float downSampledDepth = max(gatheredSamples.x, max(gatheredSamples.y, max(gatheredSamples.z, gatheredSamples.w)));
			
			downSampleDepthBuffer[pixelID.xy] = downSampledDepth;
		}
	}
}
