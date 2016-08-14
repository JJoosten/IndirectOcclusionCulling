
// INFO: ABOVE THIS LINE, ADDITIONAL DEFINES ARE PLACED


// EXECUTE INDIRECT HELPER STRUCTS
struct VertexBufferView
{
	uint2 Address;
	uint Size;
	uint Stride;
};

struct IndexBufferView
{
	uint2 Address;
	uint Size;
	uint Format;
};

struct DrawIndexedInstancedArgs
{
	uint IndexCountPerInstance;
	uint InstanceCount;
	uint StartIndexLocation;
	int  BaseVertexLocation;
	uint StartInstanceLocation;
};

struct IndirectCommandArgs
{
	IndexBufferView IndexBuffer;
	VertexBufferView VertexBuffer;
	uint modelMatrixIndex;
	uint albedoTextureIndex;
	DrawIndexedInstancedArgs DrawIndexedInstanced;
	uint _padding;
};


cbuffer rc_constants : register(b3)
{
	uint maxNumIndirectCommands;
};

StructuredBuffer<uint>							r_visibility		: register(t0); // SRV
StructuredBuffer<IndirectCommandArgs>			r_inputCommands		: register(t1);	// SRV
AppendStructuredBuffer<IndirectCommandArgs>		outputCommands		: register(u2); // UAV


[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	uint meshIndex = (groupId.x * NUM_THREADS_X) + groupIndex;

	if (meshIndex < maxNumIndirectCommands)
	{
		if (r_visibility[meshIndex] == 1)
		{
			outputCommands.Append(r_inputCommands[meshIndex]);
		}
	}
}

// vertex shader variant of compute shader written above
void VSMain(uint meshIndex : SV_VertexID)
{
	if (r_visibility[meshIndex] == 1)
	{
		outputCommands.Append(r_inputCommands[meshIndex]);
	}
}