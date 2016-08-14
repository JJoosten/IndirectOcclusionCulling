
// INFO: r_ adds a cbv, srv and uav as a root parameter
// INFO: rc_ adds a cbv as root constants

#define BIAS_CLIP_SPACE_EDGE_XY 0.9999
#define BIAS_CLIP_SPACE_EDGE_NEAR_PLANE 0.0001

// NOTE: r_ in r_cbr signals the shader reflector to add the cbv as a root parameter
cbuffer r_cbvViewInfo: register(b2)
{
	struct viewStateStruct
	{
		float4x4 ProjectionMat;
		float4x4 ViewMat;
		float2 ScreenSize;
	} viewInfo;
};

// TODO: we can potentially optimize this by making this only float3 for pos and scale, since aabb dont have rotation
StructuredBuffer<float4x4> r_modelMatrices : register(t0);
StructuredBuffer<float4x4> r_worldMatrices : register(t1);

RWStructuredBuffer<uint> r_visibility : register(u1);

struct VSInput
{
	float3 position : POSITION;
};

struct PSInput
{
	float4 position : SV_POSITION;
	nointerpolation uint objectIndex : OBJECT_INDEX;
};

// TODO: resolve instancing with this technique
PSInput VSMain(VSInput vsInput, uint objectIndex : SV_InstanceID)
{
	PSInput result;

	float3 modelPos = mul(r_modelMatrices[objectIndex], float4(vsInput.position.xyz,1.0));
	float3 worldPos = mul(r_worldMatrices[objectIndex], float4(modelPos, 1.0));

	result.position = mul(viewInfo.ProjectionMat, mul(viewInfo.ViewMat, float4(worldPos, 1.0)));

	result.objectIndex = objectIndex;

	// reset visibility
	r_visibility[objectIndex] = 0;

	return result;
}

// note that we need to set this SM5 attribute to make sure that early Z is forced, else pixel shader might run without fragments visible
[earlydepthstencil]
void PSMain(PSInput input)
{
	// if the pixel shader is invoked, the fragment is visible and we thus need to render the object
	r_visibility[input.objectIndex] = 1;
}
