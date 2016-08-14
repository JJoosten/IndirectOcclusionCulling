#pragma once

#include <cfc/base.h>
#include <cfc/core/context.h>
#include <cfc/stl/stl_pimpl.hpp>
#include <cfc/gpu/gpu.h>

struct D3D12_COMMAND_SIGNATURE_DESC;

namespace cfc {

class gpu_dx12_context;
class gpu_dx12_cmdlist_direct_api;
class gpu_dx12_cmdlist_bundle_api;
struct _imp_gpu_dx12_context;
struct _imp_gpu_dx12_indirect;

class CFC_API gpu_dx12_cmdlist_indirect_api
{
	friend class gpu_dx12_cmdlist_bundle_api;

public:
	struct dx12_indirect_command_descriptors
	{
		u64 ICRootConstantViewAddress;

		u32 ICRootConstantArgs;

		struct ICDrawInstanceArgs
		{
			u32 VertexCountPerInstance;
			u32 InstanceCount;
			u32 StartVertexLocation;
			u32 StartInstanceLocation;
		};

		struct ICDrawIndexedInstancedArgs
		{
			u32 IndexCountPerInstance;
			u32 InstanceCount;
			u32 StartIndexLocation;
			i32 BaseVertexLocation;
			u32 StartInstanceLocation;
		};

		struct ICDirectDispatchArgs
		{
			u32 ThreadGroupCountX;
			u32 ThreadGroupCountY;
			u32 ThreadGroupCountZ;
		};

		struct ICVertexBufferViewArgs
		{
			u64 BufferLocation;
			u32 SizeInBytes;
			u32 StrideInBytes;
		};

		struct ICIndexBufferViewArgs
		{
			u64 BufferLocation;
			u32 SizeInBytes;
			u32 Format;
		};
	};

public:
	gpu_dx12_cmdlist_indirect_api();
	gpu_dx12_cmdlist_indirect_api(gpu_dx12_context& ctx);
	~gpu_dx12_cmdlist_indirect_api();

	// INFO: IC stands for indirect command	
	void ICDrawInstanced();
	void ICDrawIndexedInstanced();
	void ICDispatch();
	void ICSetRoot32BitConstants(u32 RootParameterIndex, u32 Num32BitValuesToSet, u32 DestOffsetIn32BitValues);
	void ICSetRootConstantBufferView(u32 RootParameterIndex);
	void ICSetRootShaderResourceView(u32 RootParameterIndex);
	void ICSetRootUnorderedAccessView(u32 RootParameterIndex);
	void ICIASetVertexBuffers(u32 StartSlot, u32 NumViews);
	void ICIASetIndexBuffer();
	bool CompileIC(usize rootSignatureIdx = cfc::invalid_index, u32 nodeMask = CFC_GPU_NODEMASK0());

	usize GetICTemplateSizeInBytes() const { return m_templateSizeInBytes;  }

protected:
	gpu_dx12_context* m_ctx;
	u32 m_templateSizeInBytes;
	stl_pimpl<_imp_gpu_dx12_indirect, 128> m_impl;
};

class CFC_API gpu_dx12_cmdlist_bundle_api
{

public:
	gpu_dx12_cmdlist_bundle_api();
	gpu_dx12_cmdlist_bundle_api(gpu_dx12_context& ctx, usize cmdlistIndex);
	bool Reset(usize CmdAllocatorIdx, usize PipelineStateIdx = cfc::invalid_index);
	void DrawInstanced(u32 VertexCountPerInstance, u32 InstanceCount, u32 StartVertexLocation, u32 StartInstanceLocation);
	void DrawIndexedInstanced(u32 IndexCountPerInstance, u32 InstanceCount, u32 StartIndexLocation, i32 BaseVertexLocation, u32 StartInstanceLocation);
	void Dispatch(u32 ThreadGroupCountX, u32 ThreadGroupCountY, u32 ThreadGroupCountZ);
	void IASetPrimitiveTopology(gpu_primitive_type PrimitiveTopology);
	void OMSetBlendFactor(const f32 BlendFactor[4]);
	void OMSetStencilRef(u32 StencilRef);
	void SetPipelineState(usize pipelineStateIdx);
	void SetDescriptorHeaps(u32 NumDescriptorHeaps, const usize* pDescriptorHeapIdxArray);
	void SetGraphicsRootSignature(usize RootSignatureIdx);
	void SetGraphicsRootDescriptorTable(u32 RootParameterIndex, u64 GpuBaseDescriptor);
	void SetGraphicsRoot32BitConstant(u32 RootParameterIndex, u32 SrcData, u32 DestOffsetIn32BitValues);
	void SetGraphicsRoot32BitConstants(u32 RootParameterIndex, u32 Num32BitValuesToSet, const void *pSrcData, u32 DestOffsetIn32BitValues);
	void SetGraphicsRootConstantBufferView(u32 RootParameterIndex, u64 GpuBufferLocation);
	void SetGraphicsRootShaderResourceView(u32 RootParameterIndex, u64 GpuBufferLocation);
	void SetGraphicsRootUnorderedAccessView(u32 RootParameterIndex, u64 GpuBufferLocation);
	void SetComputeRootSignature(usize RootSignatureIdx);
	void SetComputeRootDescriptorTable(u32 RootParameterIndex, u64 GpuBaseDescriptor);
	void SetComputeRoot32BitConstant(u32 RootParameterIndex, u32 SrcData, u32 DestOffsetIn32BitValues);
	void SetComputeRoot32BitConstants(u32 RootParameterIndex, u32 Num32BitValuesToSet, const void *pSrcData, u32 DestOffsetIn32BitValues);
	void SetComputeRootConstantBufferView(u32 RootParameterIndex, u64 GpuBufferLocation);
	void SetComputeRootShaderResourceView(u32 RootParameterIndex, u64 GpuBufferLocation);
	void SetComputeRootUnorderedAccessView(u32 RootParameterIndex, u64 GpuBufferLocation);
	void IASetIndexBuffer(u64 GpuBufferLocation, u32 SizeInBytes, gpu_format_type Format);
	void IASetVertexBuffers(u32 StartSlot, u32 NumViews, const gpu_vertexbuffer_view *pViews);
	void ResolveQueryData(usize QueryHeapIdx, gpu_query_type Type, u32 StartIndex, u32 NumQueries, usize pDestinationBuffer, u64 AlignedDestinationBufferOffset);
	void ExecuteIndirect(const gpu_dx12_cmdlist_indirect_api* cmdListIndirect, u32 maxCommands, usize argumentBufferIdx, u64 argumentBufferOffset, usize countBufferIdx = invalid_index, u64 countBufferOffset = 0);
	bool Close();

protected:
	gpu_dx12_context* m_ctx;
	void* m_cmdlist;
};

class CFC_API gpu_dx12_cmdlist_direct_api : public gpu_dx12_cmdlist_bundle_api
{
public:
	gpu_dx12_cmdlist_direct_api();
	gpu_dx12_cmdlist_direct_api(gpu_dx12_context& ctx, usize cmdlistIndex);
	void ClearState(usize PipelineStateIdx);
	void CopyBufferRegion(usize pDstBuffer, u64 DstOffset, usize pSrcBuffer, u64 SrcOffset, u64 NumBytes);
	void CopyTextureRegion(const gpu_texturecopy_desc& pDst, const gpu_texturecopy_desc& pSrc, u32 DstX=0, u32 DstY=0, u32 DstZ=0, const gpu_box *pSrcBox=nullptr);
	void CopyResource(usize pDstResource, usize pSrcResource);
	//void CopyTiles(usize pTiledResource, const D3D12_TILED_RESOURCE_COORDINATE *pTileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE *pTileRegionSize, usize pBuffer, u64 BufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS Flags);
	void ResolveSubresource(usize pDstResource, u32 DstSubresource, usize pSrcResource, u32 SrcSubresource, gpu_format_type Format);
	void RSSetViewports(u32 NumViewports, const gpu_viewport *pViewports);
	void RSSetScissorRects(u32 NumRects, const gpu_rectangle *pRects);
	void ResourceBarrier(u32 NumBarriers, const gpu_resourcebarrier_desc *pBarriers);
	void ExecuteBundle(usize BundleCmdListIdx);
	void SOSetTargets(u32 StartSlot, u32 NumViews, const gpu_streamout_view *pViews);
	void OMSetRenderTargets(u32 NumRenderTargetDescriptors, const usize* CpuRenderTargetDescriptors, bool RTsSingleHandleToDescriptorRange, const usize CpuDepthStencilDescriptor= cfc::invalid_index);
	void ClearDepthStencilView(usize CpuDepthStencilView, bool ClearDepth, bool ClearStencil, f32 Depth, u8 Stencil, u32 NumRects, const gpu_rectangle *pRects);
	void ClearRenderTargetView(usize CpuRenderTargetView, const f32 ColorRGBA[4], u32 NumRects, const gpu_rectangle *pRects);
	void ClearUnorderedAccessViewUint(u64 GpuViewGPUHandleInCurrentHeap, usize CpuViewCPUHandle, usize ResourceIdx, const u32 Values[4], u32 NumRects, const gpu_rectangle *pRects);
	void ClearUnorderedAccessViewFloat(u64 GpuViewGPUHandleInCurrentHeap, usize CpuViewCPUHandle, usize ResourceIdx, const f32 Values[4], u32 NumRects, const gpu_rectangle *pRects);
	void DiscardResource(usize ResourceIdx);
	void DiscardResource(usize ResourceIdx, u32 NumRects, const gpu_rectangle *pRects, u32 FirstSubresource, u32 NumSubresources);
	void BeginQuery(usize QueryHeapIdx, gpu_query_type Type, u32 Index);
	void EndQuery(usize QueryHeapIdx, gpu_query_type Type, u32 Index);
	void SetPredication(usize ResourceIdx, u64 AlignedBufferOffset, bool OpEqualZero);
};

class CFC_API gpu_dx12_context
{
public:
	gpu_dx12_context();
	~gpu_dx12_context();

	bool DeviceInit(usize deviceIdx);
	bool DeviceIsInited() const;
	u32 DeviceGetNodeCount() const;
	usize DeviceGetCount();
	gpu_device_desc DeviceGetInfo(usize idx);
	gpu_features DeviceGetFeatures() const;
	bool DeviceIsMGPUEnabled() const;

	inline static usize GetSubresourceIndex(usize MipSlice, usize ArraySlice, usize PlaneSlice, usize MipLevels, usize ArraySize) { return MipSlice + (ArraySlice * MipLevels) + (PlaneSlice * MipLevels * ArraySize); }
	gpu_copyablefootprint_desc GetCopyableFootprints(const gpu_resource_desc& resDesc, u32 subresourceIdx = 0, u64 baseOffset = 0);
	usize GetDescriptorIncrementSize(gpu_descriptorheap_type type);

	usize CreateCommandQueue(gpu_commandlist_type type, u32 nodeMask=CFC_GPU_NODEMASK0(), u32 affinityMask = 0);
	usize CreateCommandAllocator(gpu_commandlist_type type, u32 affinityMask=0);
	usize CreateCommandList(gpu_commandlist_type type, usize cmdAllocatorIdx, bool startClosed = false, usize initialPipelineStateIdx=invalid_index, u32 nodeMask=CFC_GPU_NODEMASK0(), u32 affinityMask = 0);
	usize CreateFence(u64 initialValue, gpu_fenceshare_type type, u32 affinityMask = 0);
	usize CreateFenceEvent();
	usize CreateDescriptorHeap(gpu_descriptorheap_type type, u32 numDescriptors, bool shaderVisible, u32 nodeMask=CFC_GPU_NODEMASK0(), u32 affinityMask = 0);
	usize CreateQueryHeap(gpu_queryheap_type type, u32 numQueries, u32 nodeMask = CFC_GPU_NODEMASK0(), u32 affinityMask = 0);

	usize CreateSwapChain(usize cmdQueueIdx, i32 width, i32 height, void* windowHandle, bool windowed = true, gpu_swapimage_type imgType = gpu_swapimage_type::Rgba8Unorm, gpu_swapflip_type flipType = gpu_swapflip_type::Discard, i32 numBuffers = 2, i32 multisampleCount = 1, i32 multisampleQuality = 0);
	usize CreateShaderBlobCompile(const void* sourceData, usize sourceDataLength, const char* entryPoint, const char* shaderTarget, const char* sourceName = "", stl_string* outError = nullptr);
	usize CreateRootSignature(const gpu_rootsignature_desc& rootDesc, u32 affinityMask = 0);
	usize CreatePipelineState(const gpu_graphicspipelinestate_desc& gpsDesc, u32 affinityMask = 0);
	usize CreatePipelineState(const gpu_computepipelinestate_desc& cpsDesc, u32 affinityMask = 0);
	
	usize CreateCommittedResource(gpu_heap_type type, const gpu_resource_desc& resDesc, gpu_resourcestate::flag currentState, const gpu_defaultclear_desc* pDefaultClear = nullptr, u32 flags = 0, gpu_pageproperty_type pageproperties=gpu_pageproperty_type::Unknown);
	usize CreateSwapChainResource(usize swapChainIdx, u32 bufferID);
	
	// Terminology:
	// mipSlice: which mipmap (0 being the largest)
	// firstArraySlice: which array slice corresponds to index 0 (in case of Texture1_2_3DArray/TextureCube)
	// planeSlice: image plane slice (e.g. depth (0) or stencil (1) in case of combined depth/stencil buffers)
	bool CreateDescriptorRTVTexture(usize resourceRenderTargetIdx, usize cpuDescriptorOffset, gpu_format_type fmt = gpu_format_type::Unknown, u32 mipSlice = 0, u32 planeSlice = 0, u32 firstArraySlice = 0, u32 arraySize = ~0);
	bool CreateDescriptorDSVTexture(usize resourceDepthStencilIdx, usize cpuDescriptorOffset, bool readOnlyDepth=false, bool readOnlyStencil = false, gpu_format_type fmt = gpu_format_type::Unknown, u32 mipSlice = 0, u32 planeSlice = 0, u32 firstArraySlice = 0, u32 arraySize = ~0);
	bool CreateDescriptorSRVTexture(usize resourceTextureIdx, usize cpuDescriptorOffset, gpu_format_type fmt = gpu_format_type::Unknown, u32 mostDetailedMip=0, u32 mipLevels = ~0, f32 resourceMinLodClamp = 0.0f, u32 planeSlice = 0, u32 firstArraySlice = 0, u32 arraySize = ~0);
	bool CreateDescriptorUAVTexture(usize resourceTextureIdx, usize cpuDescriptorOffset, gpu_format_type fmt = gpu_format_type::Unknown, u32 mipSlice = 0, u32 planeSlice = 0, u32 firstArraySlice = 0, u32 arraySize = ~0);
	bool CreateDescriptorSRVBuffer(usize resourceBufferIdx, usize cpuDescriptorOffset, u32 structureByteStride=0, u64 firstElement = 0, u32 numElements = ~0);
	bool CreateDescriptorUAVBuffer(usize resourceBufferIdx, usize cpuDescriptorOffset, u32 structureByteStride=0, u64 firstElement = 0, u32 numElements = ~0, u64 counterOffsetInBytes = 0, usize counterResourceIdx=cfc::invalid_index);
	bool CreateDescriptorCBVBuffer(usize resourceBufferIdx, usize cpuDescriptorOffset, u64 offset=0);
	bool CreateDescriptorSampler(usize cpuDescriptorOffset, const cfc::gpu_sampler_desc& smpDesc);

	void Destroy(gpu_object_type type, usize idx);
	
	bool FenceSetName(usize fenceID, const i8* fenceName);
	bool FenceSetEventOnCompletion(usize fenceID, usize fenceEventID, u64 fenceValue);
	u64  FenceGetCompletedValue(usize fenceID);
	bool FenceEventWaitFor(usize fenceEventID, u32 timeout = ~0);

	u32  SwapchainGetCurrentBackbufferIndex(usize swapChainIdx);
	bool SwapchainPresent(usize swapChainIdx, u32 swapInterval, u32 flags);
	
	bool SwapchainResizeBuffer(usize swapChainIdx, u32 width, u32 height, u32 frameCount);
	bool SwapchainResizeTarget(usize swapChainIdx, gpu_format_type fmt, u32 width, u32 height, u32 fpsNumerator, u32 fpsDenominator);
	bool SwapchainGetFullscreenState(usize swapChainIdx);
	bool SwapchainSetFullscreenState(usize swapChainIdx, bool enabled);
	
	bool CommandAllocatorReset(usize commandAllocatorIdx);
	bool CommandQueueSignal(usize commandQueueIdx, usize fenceIdx, u64 fenceValue);
	bool CommandQueueWait(usize commandQueueIdx, usize fenceIdx, u64 fenceValue);
	bool CommandQueueExecuteCommandLists(usize commandQueueIdx, u32 numCommandLists, const usize* cmdListIdxArray);

	usize DescriptorHeapGetCPUAddressStart(usize descriptorHeapIdx);
	u64 DescriptorHeapGetGPUAddressStart(usize descriptorHeapIdx);

	
	void* ResourceMap(usize resourceIdx, bool read = false, u32 subResource = 0, usize startByteOffset = 0, usize endByteOffset = 0);
	void ResourceUnmap(usize resourceIdx, bool write = true, u32 subResource = 0, usize startByteOffset = 0, usize endByteOffset = 0);
	bool ResourceMakeResident(usize* resourceIndices, u32 numResources);
	bool ResourceEvict(usize* resourceIndices, u32 numResources);
	bool ResourceMakeResident(usize resourceIdx) { return ResourceMakeResident(&resourceIdx, 1); }
	bool ResourceEvict(usize resourceIdx) { return ResourceEvict(&resourceIdx, 1); }
	u64 ResourceGetGPUAddress(usize resourceIdx);
	gpu_heap_type ResourceGetHeapType(usize resourceIdx);
	const gpu_resource_desc& ResourceGetDesc(usize resourceIdx);
	
	bool ResourceSetName(usize resourceIdx, const i8* fenceName);
	void* ResourceGetCustom(usize resourceIdx);
	void ResourceSetCustom(usize resourceIdx, void* object);

	const gpu_shaderreflection_desc& ShaderBlobGetReflection(usize shaderIdx);

	void MGPUSwitchToNextNode();
	u32 MGPUGetCurrentNodeID();
	u32 MGPUGetAllNodeMask();

	// TODO: discuss if these are added to the GFX API?
	void SetStablePowerState(bool isStablePowerState);
	u64 GetTimeStampFrequency(usize cmdQueueIdx);

	void* DX12_GetDevice();
	void* DX12_GetCommandQueue(usize cmdqueue);
	void* DX12_GetCommandList(usize cmdlist);

	

	stl_pimpl<_imp_gpu_dx12_context, 4096> m_impl;
protected:
	bool ShaderBlobReflect(usize shaderIdx, gpu_shaderreflection_desc& output);
};

}; // end namespace cfc