#pragma once

#include <cfc/base.h>
#include <cfc/core/context.h>
#include <cfc/gpu/gpu.h>

#define GFX_MAX_TIMER_QUERY_DESCRIPTION_STRING_SIZE 256

namespace cfc {

	enum class gfx_texture_type
	{
		Texture1D,
		Texture2D,
		Texture3D,
	};

	enum class gfx_shader_type
	{
		Vertex,
		Pixel,
		Geometry,
		Hull,
		Domain,
		Compute,
		COUNT
	};

	enum class gfx_resource_type
	{
		Unknown,
		VertexBuffer,
		IndexBuffer,
		ConstantBuffer,
		SRVBuffer,
		UAVBuffer,
		CopySource,
		CopyDest,
		ResolveDest
	};

	// Multithreading guarantee: Only use this from one thread concurrently. For multi-threaded operation, keeps track of a query 
	class gfx_command_list;
	class CFC_API gfx_gpu_timer_query
	{
	public:
		void Begin(gfx_command_list* const cmdList, const char* const description);
		void Begin(gfx_command_list* const cmdList);
		void End();

		const char* const GetDescription() const { return m_description; }

		u32 GetStartIndex() const { return m_startIndex; }
		u32 GetEndIndex() const { return m_endIndex; }
	private:
		gfx_command_list* m_cmdList = nullptr;
		u32 m_startIndex = 0;
		u32 m_endIndex = 0;
		char m_description[GFX_MAX_TIMER_QUERY_DESCRIPTION_STRING_SIZE];
	};

	// Multithreading guarantee: Can be used from multiple threads.
	class CFC_API gfx_descriptor_heap
	{
	public:
		virtual usize GetIndex() = 0;

		virtual void SetSampler(usize idx, const cfc::gpu_sampler_desc& samplerDescriptor) = 0;

		virtual void SetSRVTexture(usize idx, usize resSrvTexture, gpu_format_type fmt = gpu_format_type::Unknown, u32 mostDetailedMip = 0, u32 mipLevels = ~0, f32 resourceMinLodClamp = 0.0f, u32 planeSlice = 0, u32 firstArraySlice = 0, u32 arraySize = ~0) = 0;
		virtual void SetSRVBuffer(usize idx, usize resSrvBuffer, u32 stride = 0, usize offset = 0) = 0;
		virtual void SetCBV(usize idx, usize resCbv, usize offset = 0) = 0;
		virtual void SetUAVBuffer(usize idx, usize resUav, u32 strideInBytes = 0, usize offsetInElements = 0, u32 numElements = 0, u32 counterOffsetInBytes = 0Ui64, usize counterResourceIdx = cfc::invalid_index) = 0;
		virtual void SetUAVTexture(usize idx, usize resUav, gpu_format_type fmt = gpu_format_type::Unknown, u32 mipSlice = 0, u32 planeSlice = 0, u32 firstArraySlice = 0, u32 arraySize = ~0) = 0;

		virtual u64 GetUAVCPUDescriptorHandle(usize idx) = 0;
		virtual u64 GetUAVGPUDescriptorHandle(usize idx) = 0;
	protected:
		gfx_descriptor_heap() {}
		virtual ~gfx_descriptor_heap() {}
	};

	// Multithreading guarantee: Only use this from one thread concurrently. For multi-threaded operation, create multiple instances of barrier lists.
	class CFC_API gfx_barrier_list
	{
	public:
		gfx_barrier_list(usize reserve = 4);

		void Reset();
		void BarrierResource(usize resourceIdx, u32 stateBefore, u32 stateAfter);
		void BarrierUAV(usize resourceIdx);
		void BarrierAliasing(usize resourceBeforeIdx, usize resourceAfterIdx);

	public:
		stl_vector<cfc::gpu_resourcebarrier_desc> Barriers;
	};

	// Multithreading guarantee: Only use this from one thread concurrently. For multi-threaded operation, create multiple instances of command lists.
	class CFC_API gfx_indirect_command_list
	{
	public:
		virtual usize GetIndex() const = 0;

		virtual bool Close() = 0;

		virtual void ICDrawInstanced() = 0;
		virtual void ICDrawIndexedInstanced() = 0;
		virtual void ICDispatch() = 0;
		virtual void ICSetRoot32BitConstants(u32 RootParameterIndex, u32 Num32BitValuesToSet, u32 DestOffsetIn32BitValues) = 0;
		virtual void ICSetRootConstantBufferView(u32 RootParameterIndex) = 0;
		virtual void ICSetRootShaderResourceView(u32 RootParameterIndex) = 0;
		virtual void ICSetRootUnorderedAccessView(u32 RootParameterIndex) = 0;
		virtual void ICIASetVertexBuffers(u32 StartSlot, u32 NumViews) = 0;
		virtual void ICIASetIndexBuffer() = 0;
	};

	// Multithreading guarantee: Only use this from one thread concurrently. For multi-threaded operation, create multiple instances of command lists.
	class CFC_API gfx_command_list
	{
	public:
		enum class type
		{
			Direct,
			Bundle,
		};

		virtual usize GetIndex() const = 0;
		virtual type GetType() const = 0;

		virtual void Reset() = 0;
		virtual bool Close() = 0;

		// SHARED 
		virtual void SetDescriptorHeap(gfx_descriptor_heap* heap) = 0;
		inline  void ExecuteBarrier(gfx_barrier_list& barrierList) { ExecuteBarrier(&barrierList.Barriers[0], barrierList.Barriers.size()); }
		inline  void ExecuteBarrier(cfc::gpu_resourcebarrier_desc& barrier) { ExecuteBarrier(&barrier, 1); }
		virtual void ExecuteBarrier(cfc::gpu_resourcebarrier_desc* barriers, usize count) = 0;
		virtual u32 InsertTimerQuery() = 0;
		virtual void ResolveQueryData(usize queryHeapIdx, cfc::gpu_query_type Type, u32 StartIndex, u32 NumQueries, usize destinationResourceIdx, u64 AlignedDestinationBufferOffset) = 0;

		// GRAPHICS (GFX)
		// TODO: come up with better name for GFXSetProgram (only changes root signature) where SetProgramState does the actual program switch, maybe GFXSetProgramBindings?
		virtual void GFXSetProgram(usize programIdx) = 0;
		virtual void GFXSetProgramState(usize programIdx, usize programStateIdx) = 0;
		virtual void GFXSetViewports(const cfc::gpu_viewport* viewports, usize numViewports) = 0;
		virtual void GFXSetScissorRects(const cfc::gpu_rectangle* scissorRects, usize numScissorRects) = 0;
		virtual void GFXSetRenderTargets(const usize* rtvOffsets, usize numRtvs, usize dsvOffset) = 0;
		virtual void GFXSetPrimitiveTopology(cfc::gpu_primitive_type primType) = 0;
		virtual void GFXSetVertexBuffer(i32 startSlot, usize idx, usize offset, u32 stride, u32 size) = 0;
		virtual void GFXSetIndexBuffer(usize idx, usize offset, usize sizeInBytes, cfc::gpu_format_type fmt) = 0;
		virtual void GFXSetDescriptorTableCbvSrvUav(i32 slot, usize blockIndex) = 0;
		virtual void GFXSetDescriptorTableSamplers(i32 slot, usize blockIndex) = 0;
		virtual void GFXSetRootParameterCBV(i32 slot, usize cbv, u64 offset = 0) = 0;
		virtual void GFXSetRootParameterUAV(i32 slot, usize uav, u64 offset = 0) = 0;
		virtual void GFXSetRootParameterSRV(i32 slot, usize srv, u64 offset = 0) = 0;
		virtual void GFXSetRootParameterConstants(i32 slot, const void* data, u32 sizeInDwords, u32 offsetInDwords = 0) = 0;
		virtual void GFXDrawInstanced(u32 VertexCountPerInstance, u32 InstanceCount, u32 StartVertexLocation, u32 StartInstanceLocation) = 0;
		virtual void GFXDrawIndexedInstanced(u32 IndexCountPerInstance, u32 InstanceCount, u32 StartIndexLocation, i32 BaseVertexLocation, u32 StartInstanceLocation) = 0;
		virtual void GFXExecuteBundle(usize cmdBundleIdx) = 0;
		virtual void GFXClearRenderTarget(usize rtvOffset, const float* clearColor = nullptr) = 0;
		virtual void GFXClearDepthStencilTarget(usize dsvOffset, float clearDepth = 1.0f, u8 clearStencil = 0) = 0;

		// COMPUTE (CMP)
		virtual void CMPSetProgram(usize programIdx) = 0;
		virtual void CMPSetProgramState(usize programIdx, usize programStateIdx) = 0;
		virtual void CMPSetDescriptorTableCbvSrvUav(i32 slot, usize blockIndex) = 0;
		virtual void CMPSetDescriptorTableSamplers(i32 slot, usize blockIndex) = 0;
		virtual void CMPSetRootParameterCBV(i32 slot, usize cbv, u64 offset = 0) = 0;
		virtual void CMPSetRootParameterUAV(i32 slot, usize uav, u64 offset = 0) = 0;
		virtual void CMPSetRootParameterSRV(i32 slot, usize srv, u64 offset = 0) = 0;
		virtual void CMPSetRootParameterConstants(i32 slot, const void* data, u32 sizeInDwords, u32 offsetInDwords = 0) = 0;
		virtual void CMPDispatch(u32 ThreadGroupCountX, u32 ThreadGroupCountY, u32 ThreadGroupCountZ) = 0;

		// GRAPHICS (GFX) helpers
		void GFXSetRenderTargets(usize rtvOffset, usize dsvOffset) { GFXSetRenderTargets(&rtvOffset, 1, dsvOffset); }
		void GFXSetViewports(const cfc::gpu_viewport& gpu_viewport) { GFXSetViewports(&gpu_viewport, 1); }
		void GFXSetScissorRects(const cfc::gpu_rectangle& rect) { GFXSetScissorRects(&rect, 1); }
	};

	class CFC_API gfx_texture_creation_desc
	{
	public:
		gfx_texture_creation_desc(const void* initialData, cfc::gpu_format_type fmt, int width, int height, int mipmaps=1, int arraySize=1, bool allowUAV=false)
			: Type(cfc::gfx_texture_type::Texture2D), Format(fmt), Width(width), Height(height), Mipmaps(mipmaps), ArraySize(arraySize), InitialData(initialData), AllowUAV(allowUAV) {}

		cfc::gfx_texture_type Type;
		cfc::gpu_format_type Format;
		int Width = 0;
		int Height = 1;
		int Depth = 1;
		int Mipmaps = 1;
		int ArraySize = 1;

		bool AllowUAV = false;
		bool AllowSRV = true;
		bool AllowRTV = false;
		bool AllowDSV = false;

		const void* InitialData = nullptr;
	};

	// Multithreading guarantee: Only use this from one thread concurrently. For multi-threaded operation, create multiple instances of resource streams.
	class CFC_API gfx_resource_stream
	{
	public:
		virtual usize GetIndex() = 0;

		virtual usize AddTexture(const gfx_texture_creation_desc& descriptor) = 0;
		virtual void UpdateTexture(usize resourceIdx, const void* dataBuffer, usize dataBufferRowPitch, int w, int h=1, int d=1, int dest_x = 0, int dest_y = 0, int dest_z = 0, int dest_mip = 0, int dest_arraySlice = 0) = 0;

		virtual usize AddStaticResource(gfx_resource_type type, const void* bufferData, usize bytes) = 0;
		virtual usize AddDynamicResource(gfx_resource_type type, usize bytes, bool cpuResident = false, bool readBack = false) = 0;
		virtual void UpdateDynamicResource(usize resourceIdx, u64 bytes, const void* dataBuffer, u64 dstOffset = 0) = 0;

		virtual usize AllocateTemporary(gfx_resource_type type, usize bytes, u64& outResourceOffset, u64 alignment = 256) = 0;

		virtual void Flush() = 0;
		virtual void WaitForFinish() = 0;
	};

	struct gfx_gfxprogram_desc
	{
		cfc::gpu_graphicspipelinestate_desc Pipeline;
		bool GenerateInputLayout = true;
	};

	struct gfx_cmpprogram_desc
	{
		cfc::gpu_computepipelinestate_desc Pipeline;
		bool GenerateInputLayout = true;
	};

	// Multithreading guarantee: Fully thread-safe. Can be called from any thread in any order.
	class CFC_API gfx : public cfc::object
	{
	public:
		virtual ~gfx() {}
		virtual void Init(cfc::context& ctx, usize deviceID = 0, u32 numFrames = 2, cfc::gpu_swapimage_type imgType = cfc::gpu_swapimage_type::Rgba8Unorm, cfc::gpu_format_type depthType = cfc::gpu_format_type::D32Float, cfc::gpu_swapflip_type flipType = cfc::gpu_swapflip_type::Discard) {}
		virtual gpu_features GetDeviceFeatures() const = 0;

		virtual usize AddShaderFromFile(cfc::context& ctx, const char* filename, const char* funcName = "main", const char* shaderType = "vs_5_0", const char* defineData = nullptr);
		virtual usize AddShaderFromMemory(const void* fileData, usize fileDataSize, const char* funcName = "main", const char* shaderType = "vs_5_0", const char* shaderFilename = "unknown.shd", const char* defineData = nullptr) = 0;
		virtual usize AddGraphicsProgram(usize vertexShader = cfc::invalid_index, usize pixelShader = cfc::invalid_index, usize geometryShader = cfc::invalid_index, usize hullShader = cfc::invalid_index, usize domainShader = cfc::invalid_index) = 0;
		virtual usize AddGraphicsProgramPipelineState(usize graphicsProgramIdx, const gfx_gfxprogram_desc& descriptor) = 0;
		virtual usize AddComputeProgram(usize computeShader = cfc::invalid_index) = 0;
		virtual usize AddComputeProgramPipelineState(usize computeProgramIdx, const gfx_cmpprogram_desc& descriptor) = 0;
		virtual usize AddDescriptorHeap(i32 maxCbvSrvUav = 256, i32 maxSamplers = 128) = 0;
		virtual usize AddResourceStream() = 0;
		virtual usize AddCommandList() = 0;
		virtual usize AddCommandBundle(usize cmdAllocatorBundleId) = 0;
		virtual usize AddBundleAllocator() = 0;
		virtual usize AddRenderTarget2D(i32 width, i32 height, cfc::gpu_format_type format, cfc::gpu_defaultclear_desc defaultClear = cfc::gpu_defaultclear_desc()) = 0;

		virtual gfx_descriptor_heap* GetDescriptorHeap(usize descriptorHeapIdx) = 0;
		virtual gfx_resource_stream* GetResourceStream(usize resourceStreamIdx) = 0;
		virtual gfx_command_list* GetCommandList(usize commandListIdx) = 0;
		virtual gfx_command_list* GetCommandBundle(usize bundleCmdListIdx) = 0;
		virtual usize GetBundleAllocator(usize bundleAllocatorIdx) = 0;

		virtual void RemoveShader(usize shaderID) = 0;
		virtual void RemoveResource(usize bufferID) = 0;
		virtual void RemoveResourceStream(usize resourceStream) = 0;
		virtual void RemoveDescriptorHeap(usize descriptorHeapIdx) = 0;
		virtual void RemoveCommandList(usize commandListIdx) = 0;
		virtual void RemoveCommandBundle(usize bundleCmdListIdx) = 0;
		virtual void RemoveBundleAllocator(usize bundleAllocatorIdx) = 0;
		virtual void RemoveGraphicsProgram(usize gfxProgramID) = 0;
		virtual void RemoveComputeProgram(usize cmpProgramID) = 0;
		virtual void RemoveRenderTarget(usize rtID) = 0;

		virtual void WaitForGpu() = 0;
		virtual void ExecuteCommandLists(const usize* commandListIndices, usize numCommandLists) = 0;
		virtual bool Present(u32 swapInterval = 1, u32 flags = 0) = 0;

		virtual void ResolveTimerQueries() = 0;
		virtual void ReadBackTimerQueryResults() = 0;
		virtual f64  GetTimerQueryResultInMS(const u32 index) = 0;
		virtual f64  GetTimerQueryResultInMS(const gfx_gpu_timer_query& timerQuery) = 0;

		virtual u32 GetGraphicsProgramBindLocation(usize gfxProgramIdx, const char* name) = 0;
		virtual u32 GetComputeProgramBindLocation(usize cmpProgramIdx, const char* name) = 0;

		virtual usize GetRenderTargetResource(usize rtID) = 0;
		virtual usize GetRenderTargetRTVOffset(usize rtID) = 0;
		virtual usize GetRenderTargetDSVOffset(usize rtID) = 0;
		virtual i32 GetRenderTargetWidth(usize rtID) = 0;
		virtual i32 GetRenderTargetHeight(usize rtID) = 0;
		virtual cfc::gpu_format_type GetRenderTargetFormat(usize rtID) = 0;

		virtual i32 GetBackbufferWidth() = 0;
		virtual i32 GetBackbufferHeight() = 0;
		virtual usize GetBackbufferRTResource() = 0;
		virtual usize GetBackbufferRTResource(usize index) = 0;
		virtual usize GetBackbufferDSResource() = 0;
		virtual usize GetBackbufferRTVOffset() = 0;
		virtual usize GetBackbufferDSVOffset() = 0;
		virtual gpu_format_type GetBackbufferRTVFormat() = 0;
		virtual gpu_format_type GetBackbufferDSVFormat() = 0;

		virtual usize GetBackbufferFrameIndex() = 0;
		virtual usize GetPreviousBackbufferFrameIndex() = 0;
		virtual usize GetBackbufferFrameQuantity() = 0;
		virtual u64 GetGlobalFrameIndex() = 0;

		virtual usize GetTimerQueryResolvedFrameIndex() = 0;
		virtual usize GetTimerQueryWriteFrameIndex() = 0;
		virtual usize GetTimerQueryFrameDelayQuantity() = 0;

		// helpers
		void ExecuteCommandLists(usize commandListIdx) { ExecuteCommandLists(&commandListIdx, 1); }
	};

}; // end namespace cfc