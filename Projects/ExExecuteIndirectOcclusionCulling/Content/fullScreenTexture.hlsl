
// INFO: ABOVE THIS LINE, ADDITIONAL DEFINES ARE PLACED

Texture2D<float> debugTexture	: register(t0); // SRV

SamplerState r_point_clamp : register(s1);

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uvCoords0 : TEXCOORD0;
};

PSInput VSMain(uint vertexID : SV_VertexID)
{
	PSInput output;
	output.position = float4((float)(vertexID / 2) * 4.0 - 1.0,
							 (float)(vertexID % 2) * 4.0 - 1.0, 0, 1);
	output.uvCoords0 = float2((float)(vertexID / 2) * 2.0,
								1.0 - (float)(vertexID % 2) * 2.0);
	return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float depth = debugTexture.SampleLevel(r_point_clamp, input.uvCoords0, 0);
	return float4(depth, depth, depth, 1.0);
}