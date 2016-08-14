
// INFO: r_ adds a cbv, srv and uav as a root parameter
// INFO: rc_ adds a cbv as root constants

// NOTE: r_ in r_cbr signals the shader reflector to add the cbv as a root parameter

cbuffer r_cbvViewInfo : register(b0)
{
	struct viewStateStruct
	{
		float4x4 ProjectionMat;
		float4x4 ViewMat;
		float2 ScreenSize;
	} viewInfo;
};

cbuffer rc_constants : register(b1)
{
	uint modelMatrixIndex;
	uint albedoTextureIndex;
};

// TODO: we can potentially optimize this by making this only float3 for pos and scale, since aabb dont have rotation
StructuredBuffer<float4x4> r_modelMatrices : register(t28);

Texture2D<float4> albedoTextures[26] : register(t2);

// NOTE: r_aniso8x_wrap is a static sampler declared in GFX Root Signature extraction
// NOTE: r_aniso8x_wrap samples Anisotropic (8x), UV Wrap
SamplerState r_aniso8x_wrap : register(s3);
SamplerState r_linear_wrap : register(s4);

struct VSInput
{
	float3 position : POSITION;
	float2 uvCoords0 : TEXCOORD;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uvCoords0 : TEXCOORD0;
};

PSInput VSMain( VSInput vsInput, uint instanceIndex : SV_InstanceID)
{
	PSInput vsOUT;

	const float3 worldPos = mul(r_modelMatrices[modelMatrixIndex], float4(vsInput.position, 1.0));
	vsOUT.position = mul(viewInfo.ProjectionMat, mul(viewInfo.ViewMat, float4(worldPos, 1.0)));
	vsOUT.uvCoords0 = vsInput.uvCoords0.xy;

	return vsOUT;
}

float4 PSMain(PSInput psIN) : SV_TARGET
{
	float4 outColor = albedoTextures[albedoTextureIndex].Sample(r_aniso8x_wrap, psIN.uvCoords0);

	return outColor;
}

// used for debug wire rendering
float4 PSWireFrame(PSInput psIN) : SV_TARGET
{
	float4 outColor = albedoTextures[albedoTextureIndex].Sample(r_linear_wrap, psIN.uvCoords0);
	return outColor / 0.5;
}
