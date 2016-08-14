#include "gfx_d3d12.h"
#include "gpu_d3d12.h"

#include <cfc/core/window.h>
#include <cfc/stl/stl_node.hpp>
#include <cfc/stl/stl_vector.hpp>
#include <cfc/stl/stl_array.hpp>
#include <cfc/stl/stl_map.hpp>
#include <cfc/stl/stl_threading.hpp>
#include <cfc/stl/stl_resource_collection.hpp>
#include <cfc/stl/stl_unique_ptr.hpp>
#include <cfc/stl/stl_string.hpp>
#include <cfc/stl/stl_string_advanced.hpp>

#define MICROPROFILE_GPU_TIMERS_D3D12
#include "dependencies/Microprofile/microprofile.h"
#include <cfc/core/logging.h>

#define MAX_FRAMES 16

#define MAX_NUM_GPU_TIMER_QUERIES (1<<16)
#define TIMER_QUERIES_FRAMES_DELAY 3

static float g_defaultClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

static const u32 g_shaderDefineCopyBufferSize = 32*1024; //32kb
static const u32 g_spinbufferSize = 32 * 1024 * 1024; //32MB

static const u32 g_maxRenderTargets = 2048; // yes this is big, but shadow maps... 

namespace cfc {

class gfx_dx12_program;
class gfx_dx12_desc_heap;
class gfx_dx12_resource_stream;
class gfx_dx12_command_list;
class gfx_dx12_bundle_command_allocator;
class dx12_gfx_render_target;

struct query_timer_frame
{
	u32 StartIndex;
};

struct _imp_dx12_gfx
{
	gfx_dx12* parent;
	cfc::context* ctx;
	cfc::window::event evResize;

	// gfx objects - frame related
	u32 frameIndex = 0;
	u32 nodeIndex = 0;
	u64 frameCounter = 0;
	gpu_viewport gpu_viewport;
	gpu_rectangle scissorRect;

	gpu_dx12_context gpuCtx;

	u32 numFrames;
	u32 numFramesPerNode;
	usize cmdQueue;

	// Swapchain invalidation
	usize swapChain;
	bool resized;
	bool isFullscreen;

	// Fences
	stl_array<u64, MAX_FRAMES> frameFenceCtr;
	usize fncFrame;
	usize fnevFrame;

	usize fncWaitForGpu;
	usize fnevWaitForGpu;
	u64 fnctrWaitForGpu = 1;

	// Rendertargets RTV/DSV
	usize renderTargetRTVHeap = 0;
	usize renderTargetDSVHeap = 0;

	// Backbuffer RTV/DSV
	stl_array<usize, MAX_FRAMES> resFrameRT;
	usize resFrameDepth = cfc::invalid_index;
	gpu_format_type resFrameRT_type;
	gpu_format_type resFrameDepth_type;
	usize backbufferRtvHeap;
	usize backbufferDsvHeap;
	usize rtvDescriptorSize;
	usize dsvDescriptorSize;
	
	// performance counters
	u64 gpuTimerQueryFrameCntr = 0;
	u64 gpuTimerQueryLastResolvedFrame = 0;
	std::atomic<u32> gpuTimerQueryIndex;
	usize gpuTimerQueryHeap = cfc::invalid_index;
	usize gpuTimerQueryReadbackBuffer = cfc::invalid_index;
	usize gpuTimerQueryFnc = cfc::invalid_index;
	usize gpuTimerQueryFnev = cfc::invalid_index;
	u64 gpuTimerQueryTimeStampFrequency = 0;
	stl_vector<u64> gpuTimerQueryResults;
	stl_array<usize, TIMER_QUERIES_FRAMES_DELAY> gpuTimerQueryResolveCommandLists;
	stl_array<query_timer_frame, TIMER_QUERIES_FRAMES_DELAY> gpuTimerQueryFrameInfo;

	// Deletion Queue
	stl_mutex mtxDeletionQueue;
	stl_map<u64, stl_vector<stl_pair<gpu_object_type, usize> > > deletionQueue;

	// Resources
	stl_resource_collection<stl_unique_ptr<gfx_dx12_program>, stl_mutex> resGfxPrograms;
	stl_resource_collection<stl_unique_ptr<gfx_dx12_desc_heap>, stl_mutex> resDescriptorHeaps;
	stl_resource_collection<stl_unique_ptr<gfx_dx12_resource_stream>, stl_mutex> resResourceStreams;
	stl_resource_collection<stl_unique_ptr<gfx_dx12_command_list>, stl_mutex> resCommandLists;
	stl_resource_collection<stl_unique_ptr<gfx_dx12_command_list>, stl_mutex> resCommandBundles;
	stl_resource_collection<stl_unique_ptr<dx12_gfx_render_target>, stl_mutex> resRenderTargets;

	stl_resource_collection<usize, stl_mutex> resBundleCommandAllocator;
};

// ** State API
class gfx_dx12_resource_stream;

class gfx_dx12_program
{
public:
	gfx_dx12_program(_imp_dx12_gfx* impl) : m_impl(impl){}
	virtual	~gfx_dx12_program()
	{
		// cleanup
		m_impl->parent->QueueDestroy(gpu_object_type::RootSignature, rootSignatureIndex);
		for(auto idx: graphicsPipelineStateIndices)
			m_impl->parent->QueueDestroy(gpu_object_type::PipelineState, idx);
	}

	_imp_dx12_gfx* m_impl;
	usize m_index;
	cfc::gpu_rootsignature_desc rootSignatureDescriptor;
	stl_array<usize, 6> shaderIndices;
	usize rootSignatureIndex;
	stl_vector<usize> graphicsPipelineStateIndices;
	// TODO: hash string
	stl_map<stl_string, usize> shaderBindings;
};

class gfx_dx12_desc_heap : public gfx_descriptor_heap
{
public:
	gfx_dx12_desc_heap(_imp_dx12_gfx* ctx) { m_impl = ctx; }
	virtual ~gfx_dx12_desc_heap()
	{
		// cleanup pools
		for (int i = 0; i < 2; i++)
			m_impl->parent->QueueDestroy(gpu_object_type::DescriptorHeap, m_heapInfo[i].heap);

		numHeaps = 0;
	}

	virtual usize GetIndex() override
	{
		return m_index;
	}

	virtual void SetSampler(usize idx, const cfc::gpu_sampler_desc& samplerDesc) override
	{
		m_impl->gpuCtx.CreateDescriptorSampler(GetCPUOffsetSamplers(idx), samplerDesc);
	}

	virtual void SetSRVTexture(usize idx, usize resSrvTexture, gpu_format_type fmt = gpu_format_type::Unknown, u32 mostDetailedMip = 0, u32 mipLevels = ~0, f32 resourceMinLodClamp = 0.0f, u32 planeSlice = 0, u32 firstArraySlice = 0, u32 arraySize = ~0) override
	{
		m_impl->gpuCtx.CreateDescriptorSRVTexture(resSrvTexture, GetCPUOffsetCBVSRVUAV(idx), fmt, mostDetailedMip, mipLevels, resourceMinLodClamp, planeSlice, firstArraySlice, arraySize);
	}

	virtual void SetSRVBuffer(usize idx, usize resSrvBuffer, u32 stride = 0, usize offset = 0) override
	{
		m_impl->gpuCtx.CreateDescriptorSRVBuffer(resSrvBuffer, GetCPUOffsetCBVSRVUAV(idx), stride, offset);
	}

	virtual void SetCBV(usize idx, usize resCbv, usize offset = 0) override
	{
		m_impl->gpuCtx.CreateDescriptorCBVBuffer(resCbv, GetCPUOffsetCBVSRVUAV(idx), offset);
	}

	virtual void SetUAVBuffer(usize idx, usize resUav, u32 strideInBytes /*= 0*/, usize offsetInElements /*= 0*/, u32 numElements /*= 0*/, u32 counterOffsetInBytes /*= 0Ui64*/, usize counterResourceIdx/*=cfc::invalid_index*/) override
	{
		m_impl->gpuCtx.CreateDescriptorUAVBuffer(resUav, GetCPUOffsetCBVSRVUAV(idx), strideInBytes, offsetInElements, numElements, counterOffsetInBytes, counterResourceIdx);
		m_impl->gpuCtx.CreateDescriptorUAVBuffer(resUav, GetUAVCPUDescriptorHandle(idx), strideInBytes, offsetInElements, numElements, counterOffsetInBytes, counterResourceIdx);
	}

	virtual void SetUAVTexture(usize idx, usize resUav, gpu_format_type fmt = gpu_format_type::Unknown, u32 mipSlice = 0, u32 planeSlice = 0, u32 firstArraySlice = 0, u32 arraySize = ~0) override
	{
		m_impl->gpuCtx.CreateDescriptorUAVTexture(resUav, GetCPUOffsetCBVSRVUAV(idx), fmt, mipSlice, planeSlice, firstArraySlice, arraySize);
		m_impl->gpuCtx.CreateDescriptorUAVTexture(resUav, GetUAVCPUDescriptorHandle(idx), fmt, mipSlice, planeSlice, firstArraySlice, arraySize);
	}


	virtual u64 GetUAVCPUDescriptorHandle(usize idx) override
	{
		return m_heapInfo[HeapUavCpu].cpuStart + m_heapInfo[HeapUavCpu].descriptorSize*idx;
	}

	virtual u64 GetUAVGPUDescriptorHandle(usize idx) override
	{
		return GetGPUOffsetCBVSRVUAV(idx);
	}

	usize GetCPUOffsetCBVSRVUAV(usize offset)
	{
		return m_heapInfo[HeapCbvSrvUav].cpuStart + m_heapInfo[HeapCbvSrvUav].descriptorSize*offset;
	}

	usize GetCPUOffsetSamplers(usize offset)
	{
		return m_heapInfo[HeapSamplers].cpuStart + m_heapInfo[HeapSamplers].descriptorSize*offset;
	}

	u64 GetGPUOffsetCBVSRVUAV(usize offset)
	{
		return m_heapInfo[HeapCbvSrvUav].gpuStart + m_heapInfo[HeapCbvSrvUav].descriptorSize*offset;
	}

	u64 GetGPUOffsetSamplers(usize offset)
	{
		return m_heapInfo[HeapSamplers].gpuStart + m_heapInfo[HeapSamplers].descriptorSize*offset;
	}

	enum  heapEnum
	{
		HeapCbvSrvUav=0,
		HeapSamplers=1,
		HeapUavCpu=2
	};

	struct heapInfo
	{
		usize heap = cfc::invalid_index;
		usize offset = 0;
		usize size = 0;
		usize descriptorSize = cfc::invalid_index;
		u64 gpuStart = ~0ULL;
		usize cpuStart = cfc::invalid_index;
	} m_heapInfo[3];

	usize heaps[2];
	u32 numHeaps = 0U;

	gfx_dx12_resource_stream* m_resStream;
	_imp_dx12_gfx* m_impl;
	usize m_index;


};

class gfx_dx12_resource_stream : public gfx_resource_stream
{
public:
	// mass barriers
	stl_vector<cfc::gpu_resourcebarrier_desc> bufferedPreBarriers;
	stl_vector<cfc::gpu_resourcebarrier_desc> bufferedPostBarriers;
	
	// in-between operations
	stl_vector<std::function<void()>> bufferedOperations;

	gfx_dx12_resource_stream(_imp_dx12_gfx* impl) 
	{ 
		m_impl = impl; 

		// initialize resource command list
		cmdResourceAllocatorFront = m_impl->gpuCtx.CreateCommandAllocator(gpu_commandlist_type::Direct, m_impl->gpuCtx.MGPUGetAllNodeMask());
		cmdResourceAllocatorBack = m_impl->gpuCtx.CreateCommandAllocator(gpu_commandlist_type::Direct, m_impl->gpuCtx.MGPUGetAllNodeMask());
		cmdResourceListIdx = m_impl->gpuCtx.CreateCommandList(gpu_commandlist_type::Direct, cmdResourceAllocatorFront, false, cfc::invalid_index, 0, m_impl->gpuCtx.MGPUGetAllNodeMask());
		gpuResourceCmds = gpu_dx12_cmdlist_direct_api(m_impl->gpuCtx, cmdResourceListIdx);

		// fences
		fncResource = m_impl->gpuCtx.CreateFence(0, gpu_fenceshare_type::Unshared);
		fnevResource = m_impl->gpuCtx.CreateFenceEvent();
		m_impl->gpuCtx.FenceSetName(fncResource, "ResourceStreamFence");
	}

	virtual ~gfx_dx12_resource_stream()
	{
		m_impl->parent->QueueDestroy(gpu_object_type::Resource, resCPUSpinheap);
		m_impl->parent->QueueDestroy(gpu_object_type::CommandAllocator, cmdResourceAllocatorFront);
		m_impl->parent->QueueDestroy(gpu_object_type::CommandAllocator, cmdResourceAllocatorBack);
		m_impl->parent->QueueDestroy(gpu_object_type::CommandList, cmdResourceListIdx);
	}

	struct supplemental_res_data
	{
		gfx_resource_type type;
		void* mappedPtr;
		bool transitionQueued;
	};

	bool logShaderError(stl_string& error, bool success)
	{
		if (success == false)
			printf("shader compilation failure:\n%s\n", error.c_str());
		return success;
	}

	u32 ResourceTypeToState(gfx_resource_type bufType)
	{
		if (bufType == gfx_resource_type::ConstantBuffer)
			return (gpu_resourcestate::VertexAndConstantBuffer);
		else if (bufType == gfx_resource_type::IndexBuffer)
			return  (gpu_resourcestate::IndexBuffer);
		else if (bufType == gfx_resource_type::VertexBuffer)
			return (gpu_resourcestate::VertexAndConstantBuffer);
		else if (bufType == gfx_resource_type::SRVBuffer)
			return (gpu_resourcestate::PixelShaderResource | gpu_resourcestate::NonPixelShaderResource);
		else if (bufType == gfx_resource_type::UAVBuffer)
			return (gpu_resourcestate::UnorderedAccess);
		else if (bufType == gfx_resource_type::CopySource)
			return (gpu_resourcestate::CopySource);
		else if (bufType == gfx_resource_type::CopyDest)
			return (gpu_resourcestate::CopyDestination);
		else if (bufType == gfx_resource_type::ResolveDest)
			return (gpu_resourcestate::ResolveDestination);
		return gpu_resourcestate::GenericRead;
	}

	supplemental_res_data& GetSupplementalResData(usize resourceIdx)
	{
		return *(supplemental_res_data*)m_impl->gpuCtx.ResourceGetCustom(resourceIdx);
	}

	virtual usize GetIndex() override
	{
		return m_index;
	}

	virtual usize AddStaticResource(gfx_resource_type bufType, const void* bufData, usize bytes) override
	{
		usize gpuResource = AddDynamicResource(bufType, bytes, false, false);
		UpdateDynamicResource(gpuResource, bytes, bufData, 0);
		return gpuResource;
	}

	virtual usize AddDynamicResource(gfx_resource_type type, usize bytes, bool cpuResident, bool readBack) override
	{
		MICROPROFILE_SCOPEI("DX12", "AddDynamicResource", 0);

		// constant buffers need to be 256 byte aligned
		if(type == gfx_resource_type::ConstantBuffer)
			bytes = (bytes + 255) & ~255;

		gpu_resource_desc dsc = gpu_resource_desc::Buffer(bytes);
		if (type == gfx_resource_type::UAVBuffer)
			dsc.Flags = gpu_resource_desc::resourceflags::AllowUnorderedAccess;

		usize res;
		if(cpuResident)
			res	= m_impl->gpuCtx.CreateCommittedResource(gpu_heap_type::Upload, dsc, gpu_resourcestate::GenericRead);
		else if (readBack)
			res = m_impl->gpuCtx.CreateCommittedResource(gpu_heap_type::Readback, dsc, ResourceTypeToState(type));
		else
			res = m_impl->gpuCtx.CreateCommittedResource(gpu_heap_type::Default, dsc, ResourceTypeToState(type));

		// add supplemental data block & null it
		m_impl->gpuCtx.ResourceSetCustom(res, malloc(sizeof(supplemental_res_data)));
		memset(&GetSupplementalResData(res), 0, sizeof(supplemental_res_data));

		// store resource type
		GetSupplementalResData(res).type = type;

		// obtain persistent mapped DMA pointer to upload heap
		if(cpuResident)
			GetSupplementalResData(res).mappedPtr = m_impl->gpuCtx.ResourceMap(res, false, 0);

		return res;
	}
	

	virtual usize AddTexture(const gfx_texture_creation_desc& desc) override
	{
		MICROPROFILE_SCOPEI("DX12", "AddTexture", 0);
		gpu_resource_desc resDesc;
		gpu_resource_desc::resourceflags::flag flags = gpu_resource_desc::resourceflags::None;
		flags |= desc.AllowUAV ? gpu_resource_desc::resourceflags::AllowUnorderedAccess : 0;
		flags |= desc.AllowSRV ? 0 : gpu_resource_desc::resourceflags::DenyShaderResource;
		flags |= desc.AllowRTV ? gpu_resource_desc::resourceflags::AllowRenderTarget : 0;
		flags |= desc.AllowDSV ? gpu_resource_desc::resourceflags::AllowDepthStencil : 0;


		switch (desc.Type)
		{
		case gfx_texture_type::Texture1D:				resDesc = gpu_resource_desc::Tex1D(desc.Format, desc.Width, desc.ArraySize, desc.Mipmaps, flags); break;
		case gfx_texture_type::Texture2D:				resDesc = gpu_resource_desc::Tex2D(desc.Format, desc.Width, desc.Height, desc.ArraySize, desc.Mipmaps, flags); break;
		case gfx_texture_type::Texture3D:				resDesc = gpu_resource_desc::Tex3D(desc.Format, desc.Width, desc.Height, desc.Depth, desc.Mipmaps, flags); break;
		}

		usize res = m_impl->gpuCtx.CreateCommittedResource(gpu_heap_type::Default, resDesc, desc.InitialData == nullptr ? ResourceTypeToState(gfx_resource_type::SRVBuffer) : cfc::gpu_resourcestate::CopyDestination);

		if (desc.InitialData)
		{
			// calculate number of footprints
			usize numFootprints = desc.Mipmaps * desc.ArraySize;

			// calculate size requirements
			stl_vector< gpu_copyablefootprint_desc> footprints;
			for (int i = 0; i < numFootprints; i++)
			{
				gpu_copyablefootprint_desc footprint;
				u64 offset = i - 1 >= 0 ? footprints[i - 1].Offset + footprints[i - 1].TotalBytes : 0;
				offset = stl_math_iroundup(offset, 512); // dx12 padding requirement
				footprint = m_impl->gpuCtx.GetCopyableFootprints(resDesc, i, offset);

				if (footprint.TotalBytes != (~0ULL))
					footprints.push_back(footprint);
				else
					break;
			}

			// create upload buffer
			usize resUpload = m_impl->gpuCtx.CreateCommittedResource(gpu_heap_type::Upload, gpu_resource_desc::Buffer(footprints.back().Offset + footprints.back().TotalBytes), gpu_resourcestate::GenericRead);
			if (resUpload == cfc::invalid_index)
				return cfc::invalid_index;

			//fill in with data
			const char* sptr = (const char*)desc.InitialData;
			char* bptr = (char*)m_impl->gpuCtx.ResourceMap(resUpload);
			for (usize srIndex = 0; srIndex < footprints.size(); srIndex++)
			{
				char* ptr = bptr + footprints[srIndex].Offset;
				for (u32 z = 0; z < footprints[srIndex].Depth; z++)
				{
					for (u32 y = 0; y < footprints[srIndex].Height; y++)
					{
						memcpy(ptr, sptr, (usize)footprints[srIndex].RowSizeInBytes);
						sptr += footprints[srIndex].RowSizeInBytes;
						ptr += footprints[srIndex].RowPitch;
					}
				}
			}
			m_impl->gpuCtx.ResourceUnmap(resUpload);

			// GPU calls
			gpu_dx12_cmdlist_direct_api& api = gpuResourceCmds;
			for (usize i = 0; i < footprints.size(); i++)
				gpuResourceCmds.CopyTextureRegion(gpu_texturecopy_desc::AsSubresourceIndex(res, (u32)i), gpu_texturecopy_desc::AsPlacedFootprint(resUpload, footprints[i]));
			gpuResourceCmds.ResourceBarrier(1, &gpu_resourcebarrier_desc::Transition(res, gpu_resourcestate::CopyDestination, ResourceTypeToState(gfx_resource_type::SRVBuffer)));
			cmdResourcesFreeListBack.push_back(stl_pair<gpu_object_type, usize>(gpu_object_type::Resource, resUpload));
		}

		return res;
	}

	virtual void UpdateDynamicResource(usize resourceIdx, u64 bytes, const void* dataBuffer, u64 dstOffset = 0)
	{
		auto heapType = m_impl->gpuCtx.ResourceGetHeapType(resourceIdx);
		if (heapType == gpu_heap_type::Upload)
		{
			char* ptr = (char*)GetSupplementalResData(resourceIdx).mappedPtr;
			memcpy(ptr + dstOffset, dataBuffer, bytes);
		}
		else if (heapType == gpu_heap_type::Default)
		{
#define METHOD_SPINHEAP_COPY_BUFFERED
#if defined(METHOD_CREATE_COPY_IMMEDIATE)
			// get resource type 
			auto resType = ResourceTypeToState(GetSupplementalResData(resourceIdx).type);

			// very inefficient - creates/destroys constant buffer every time
			usize cpuResource = AddDynamicResource(gfx_resource_type::ConstantBuffer, bytes, true);
			UpdateDynamicResource(cpuResource, bytes, dataBuffer, 0);
			gpuResourceCmds.ResourceBarrier(1, &cfc::gpu_resourcebarrier_desc::Transition(resourceIdx, resType, gpu_resourcestate::CopyDestination));
			gpuResourceCmds.CopyBufferRegion(resourceIdx, dstOffset, cpuResource, 0, bytes);
			gpuResourceCmds.ResourceBarrier(1, &cfc::gpu_resourcebarrier_desc::Transition(resourceIdx, gpu_resourcestate::CopyDestination, resType));
			cmdResourcesFreeListBack.push_back(stl_pair<gpu_object_type, usize>(gpu_object_type::Resource, cpuResource));
#elif defined(METHOD_SPINHEAP_COPY_IMMEDIATE) || defined(METHOD_SPINHEAP_COPY_BUFFERED)
			// update spinheap
			u64 spinOffset = ~0ULL;
			usize resourceTemporary = AllocateTemporary(gfx_resource_type::Unknown, bytes, spinOffset, 256);
			UpdateDynamicResource(resourceTemporary, bytes, dataBuffer, spinOffset);
			
			// get resource type 
			auto resType = ResourceTypeToState(GetSupplementalResData(resourceIdx).type);

#  if defined(METHOD_SPINHEAP_COPY_IMMEDIATE)
			// queue copy into GPU resident buffer
			gpuResourceCmds.ResourceBarrier(1, &cfc::gpu_resourcebarrier_desc::Transition(resourceIdx, resType, gpu_resourcestate::CopyDestination));
			gpuResourceCmds.CopyBufferRegion(resourceIdx, dstOffset, resCPUSpinheap, spinOffset, bytes);
			gpuResourceCmds.ResourceBarrier(1, &cfc::gpu_resourcebarrier_desc::Transition(resourceIdx, gpu_resourcestate::CopyDestination, resType));
#  elif defined(METHOD_SPINHEAP_COPY_BUFFERED)
			if(GetSupplementalResData(resourceIdx).transitionQueued == false)
			{
				bufferedPreBarriers.push_back(cfc::gpu_resourcebarrier_desc::Transition(resourceIdx, resType, gpu_resourcestate::CopyDestination));
				bufferedPostBarriers.push_back(cfc::gpu_resourcebarrier_desc::Transition(resourceIdx, gpu_resourcestate::CopyDestination, resType));
				GetSupplementalResData(resourceIdx).transitionQueued = true;
			}
			bufferedOperations.push_back([resourceIdx, dstOffset, this, spinOffset, bytes]() { 
				gpuResourceCmds.CopyBufferRegion(resourceIdx, dstOffset, resCPUSpinheap, spinOffset, bytes); 
				GetSupplementalResData(resourceIdx).transitionQueued = false; 
			});
#  endif
#endif
		}
	}

	virtual void UpdateTexture(usize resourceIdx, const void* dataBuffer, usize dataBufferRowPitch, int w, int h, int d, int dest_x, int dest_y, int dest_z, int dest_mip, int dest_arraySlice)
	{
		usize res = resourceIdx;
		gpu_resource_desc rdesc = m_impl->gpuCtx.ResourceGetDesc(resourceIdx);
		gpu_copyablefootprint_desc footprint;

		// override width/height & calculate footprint
		rdesc.Width = w;
		rdesc.Height = h;
		rdesc.DepthOrArraySize = d;
		footprint = m_impl->gpuCtx.GetCopyableFootprints(rdesc, 0, 0);

		// create upload buffer
		//usize resUpload = m_impl->gpuCtx.CreateCommittedResource(gpu_heap_type::Upload, gpu_resource_desc::Buffer(footprint.Offset + footprint.TotalBytes), gpu_resourcestate::GenericRead);
		usize resUpload = AllocateTemporary(cfc::gfx_resource_type::Unknown, footprint.TotalBytes, footprint.Offset, 512);
		if (resUpload == cfc::invalid_index)
			return;

		//fill in with data
		const char* sptr = (const char*)dataBuffer;
		char* bptr = (char*)m_impl->gpuCtx.ResourceMap(resUpload);
		char* ptr = bptr + footprint.Offset;
		for (u32 z = 0; z < footprint.Depth; z++)
		{
			for (u32 y = 0; y < footprint.Height; y++)
			{
				if (sptr != nullptr)
				{
					memcpy(ptr, sptr, (usize)footprint.RowSizeInBytes);
					sptr += dataBufferRowPitch;
				}
				else
				{
					memset(ptr, 0, (usize)footprint.RowSizeInBytes);
				}
				ptr += footprint.RowPitch;
			}
		}
		m_impl->gpuCtx.ResourceUnmap(resUpload);

		// GPU calls
		usize dest_subres = m_impl->gpuCtx.GetSubresourceIndex(dest_mip, dest_arraySlice, 0, rdesc.MipLevels, rdesc.Dimension == gpu_resource_desc::dimension::Texture3D?1:rdesc.DepthOrArraySize);
		gpu_dx12_cmdlist_direct_api& api = gpuResourceCmds;
		gpuResourceCmds.ResourceBarrier(1, &gpu_resourcebarrier_desc::Transition(res, ResourceTypeToState(gfx_resource_type::SRVBuffer), gpu_resourcestate::CopyDestination));
		gpuResourceCmds.CopyTextureRegion(gpu_texturecopy_desc::AsSubresourceIndex(res, (u32)dest_subres), gpu_texturecopy_desc::AsPlacedFootprint(resUpload, footprint), dest_x, dest_y, dest_z, nullptr);
		gpuResourceCmds.ResourceBarrier(1, &gpu_resourcebarrier_desc::Transition(res, gpu_resourcestate::CopyDestination, ResourceTypeToState(gfx_resource_type::SRVBuffer)));
	}


	usize AllocateTemporary(gfx_resource_type type, usize bytes, u64& outResourceOffset, u64 alignment)
	{
		// create spinheap if it doesn't exist yet
		if (resCPUSpinheap == cfc::invalid_index)
		{
			resCPUSpinheap_size = g_spinbufferSize;	
			resCPUSpinheap = AddDynamicResource(gfx_resource_type::Unknown, resCPUSpinheap_size, true, false);
		}

		// check size
		stl_assert(bytes <= resCPUSpinheap_size);

		// align offset
		u64 alignCs = resCPUSpinheap_offset % alignment;
		if (alignCs != 0)
		{
			resCPUSpinheap_offset += alignment - alignCs;
		}

		// check whether we need to loop around
		if (resCPUSpinheap_offset + bytes > resCPUSpinheap_size)
			resCPUSpinheap_offset = 0; // reset spin heap

		// "allocate" bytes and record offset
		outResourceOffset = resCPUSpinheap_offset;
		resCPUSpinheap_offset += bytes;

		return resCPUSpinheap;
	}

	virtual void Flush() override
	{
		MICROPROFILE_SCOPEI("DX12", "FlushResources", 0);

		// execute buffered operations
		{
			// execute pre barriers
			if (bufferedPreBarriers.size() > 0)
			{
				gpuResourceCmds.ResourceBarrier((u32)bufferedPreBarriers.size(), &bufferedPreBarriers[0]);
				bufferedPreBarriers.resize(0);
			}

			// execute buffered operations
			for (auto& operation : bufferedOperations)
				operation();

			// clear buffered operations
			bufferedOperations.resize(0);

			// execute post barriers
			if (bufferedPostBarriers.size() > 0)
			{
				gpuResourceCmds.ResourceBarrier((u32)bufferedPostBarriers.size(), &bufferedPostBarriers[0]);
				bufferedPostBarriers.resize(0);
			}
		}

		// wait for last queue to be finished
		WaitForFinish();

		// destroy front resources
		for (auto& resDelete : cmdResourcesFreeListFront)
			m_impl->gpuCtx.Destroy(resDelete.first, resDelete.second);
		
		// swap front/back destroy queues
		cmdResourcesFreeListFront.resize(0);
		cmdResourcesFreeListBack.swap(cmdResourcesFreeListFront);

		// flush new command queue
		{
			MICROPROFILE_SCOPEI("DX12", "FlushResourcesLocked", 0);

			// close resource command buffer
			stl_assert(gpuResourceCmds.Close());

			// execute resource build step, then frame commands
			usize cmdLists[] = { cmdResourceListIdx };
			stl_assert(m_impl->gpuCtx.CommandQueueExecuteCommandLists(m_impl->cmdQueue, sizeof(cmdLists) / sizeof(cmdLists[0]), cmdLists));

			// signal fence that we're ready for the next command allocator
			m_impl->gpuCtx.CommandQueueSignal(m_impl->cmdQueue, fncResource, ++resourceFrameIndex);

			// reset next allocator buffer
			stl_assert(m_impl->gpuCtx.CommandAllocatorReset(cmdResourceAllocatorBack));
			stl_assert(gpuResourceCmds.Reset(cmdResourceAllocatorBack));

			// swap resource allocators
			std::swap(cmdResourceAllocatorBack, cmdResourceAllocatorFront);
		}
	}

	virtual void WaitForFinish() override
	{
		// if previous buffer/allocator is still in use, we cannot swap yet and have to wait..
		if (m_impl->gpuCtx.FenceGetCompletedValue(fncResource) != resourceFrameIndex)
		{
			MICROPROFILE_SCOPEI("DX12", "FlushResourcesWait", 0);
			m_impl->gpuCtx.FenceSetEventOnCompletion(fncResource, fnevResource, resourceFrameIndex);
			m_impl->gpuCtx.FenceEventWaitFor(fnevResource);
		}
	}



	_imp_dx12_gfx* m_impl;
	usize m_index;

	usize cmdResourceListIdx;
	usize cmdResourceAllocatorFront, cmdResourceAllocatorBack;
	stl_vector<stl_pair<gpu_object_type, usize>> cmdResourcesFreeListBack, cmdResourcesFreeListFront;

	gpu_dx12_cmdlist_direct_api gpuResourceCmds;
	usize fncResource;
	usize fnevResource;
	u64 resourceFrameIndex = 0;

	// Spinheap
	usize resCPUSpinheap = cfc::invalid_index;
	u64 resCPUSpinheap_offset = 0;
	u64 resCPUSpinheap_size = 0;

};

class gfx_dx12_command_list : public gfx_command_list
{
public:
	gfx_dx12_command_list(_imp_dx12_gfx* impl, type cmdListType, usize cmdAllocatorBundleId, usize nodeMask = CFC_GPU_NODEMASK0())
	{
		m_impl = impl;
		m_type = cmdListType;

		if (m_type == type::Direct)
		{
			cmdAllocatorIdx = m_impl->gpuCtx.CreateCommandAllocator(gpu_commandlist_type::Direct);
			cmdListIdx = m_impl->gpuCtx.CreateCommandList(gpu_commandlist_type::Direct, cmdAllocatorIdx, true, cfc::invalid_index, nodeMask);
			cmdListDirectApi = gpu_dx12_cmdlist_direct_api(m_impl->gpuCtx, cmdListIdx);
			cmdListBundleApi = gpu_dx12_cmdlist_bundle_api(m_impl->gpuCtx, cmdListIdx);
		}
		else
		{
			cmdAllocatorIdx = cmdAllocatorBundleId;
			cmdListIdx = m_impl->gpuCtx.CreateCommandList(gpu_commandlist_type::Bundle, cmdAllocatorIdx, true, cfc::invalid_index, nodeMask);
			cmdListBundleApi = gpu_dx12_cmdlist_bundle_api(m_impl->gpuCtx, cmdListIdx);
		}
	}

	virtual ~gfx_dx12_command_list()
	{
		if (m_type == type::Direct)
		{
			m_impl->parent->QueueDestroy(gpu_object_type::CommandAllocator, cmdAllocatorIdx);
		}
		
		m_impl->parent->QueueDestroy(gpu_object_type::CommandList, cmdListIdx);
	}

	virtual type GetType() const override
	{
		return m_type;
	}

	virtual usize GetIndex() const override
	{
		return m_index;
	}

	usize GetCommandListIndex() const { return cmdListIdx; }

	virtual void Reset() override
	{
		m_impl->gpuCtx.CommandAllocatorReset(cmdAllocatorIdx);
		cmdListBundleApi.Reset(cmdAllocatorIdx);
	}

	virtual bool Close() override
	{
		return cmdListBundleApi.Close();
	}


	virtual void SetDescriptorHeap(gfx_descriptor_heap* heap) override
	{
		gfx_dx12_desc_heap* rHeap = reinterpret_cast<gfx_dx12_desc_heap*>(heap);
		cmdListBundleApi.SetDescriptorHeaps(rHeap->numHeaps, rHeap->heaps);
		activeHeap = rHeap;
	}

	virtual void GFXSetProgram(usize gfxProgramIdx) override
	{
		gfx_dx12_program* program = m_impl->resGfxPrograms[gfxProgramIdx].get();

		cmdListBundleApi.SetGraphicsRootSignature(program->rootSignatureIndex);
	}

	virtual void GFXSetProgramState(usize gfxProgramIdx, usize programStateIdx) override
	{
		gfx_dx12_program* program = m_impl->resGfxPrograms[gfxProgramIdx].get();
		cmdListBundleApi.SetPipelineState(program->graphicsPipelineStateIndices[programStateIdx]);
	}

	virtual void GFXSetDescriptorTableCbvSrvUav(i32 slot, usize blockIndex) override
	{
		cmdListBundleApi.SetGraphicsRootDescriptorTable(slot, activeHeap->GetGPUOffsetCBVSRVUAV(blockIndex));
	}

	virtual void GFXSetDescriptorTableSamplers(i32 slot, usize blockIndex) override
	{
		cmdListBundleApi.SetGraphicsRootDescriptorTable(slot, activeHeap->GetGPUOffsetSamplers(blockIndex));
	}

	virtual void GFXSetRootParameterCBV(i32 slot, usize cbv, u64 offset) override
	{
		cmdListBundleApi.SetGraphicsRootConstantBufferView(slot, m_impl->gpuCtx.ResourceGetGPUAddress(cbv) + offset);
	}

	virtual void GFXSetRootParameterUAV(i32 slot, usize uav, u64 offset) override
	{
		cmdListBundleApi.SetGraphicsRootUnorderedAccessView(slot, m_impl->gpuCtx.ResourceGetGPUAddress(uav) + offset);
	}

	virtual void GFXSetRootParameterSRV(i32 slot, usize srv, u64 offset) override
	{
		cmdListBundleApi.SetGraphicsRootShaderResourceView(slot, m_impl->gpuCtx.ResourceGetGPUAddress(srv) + offset);
	}

	virtual void GFXSetRootParameterConstants(i32 slot, const void* data, u32 sizeInDwords, u32 offsetInDwords = 0) override
	{
		cmdListBundleApi.SetGraphicsRoot32BitConstants(slot, sizeInDwords, data, offsetInDwords);
	}

	virtual void GFXSetPrimitiveTopology(cfc::gpu_primitive_type primType) override
	{
		cmdListBundleApi.IASetPrimitiveTopology(primType);
	}

	virtual void GFXSetVertexBuffer(i32 startSlot, usize idx, usize offset, u32 stride, u32 size) override
	{
		cfc::gpu_vertexbuffer_view vbv;
		vbv.GpuBufferLocation = m_impl->gpuCtx.ResourceGetGPUAddress(idx) + offset;
		vbv.SizeInBytes = size;
		vbv.StrideInBytes = stride;
		cmdListBundleApi.IASetVertexBuffers(startSlot, 1, &vbv);
	}

	virtual void GFXSetIndexBuffer(usize idx, usize offset, usize sizeInBytes, cfc::gpu_format_type fmt) override
	{
		cmdListBundleApi.IASetIndexBuffer(m_impl->gpuCtx.ResourceGetGPUAddress(idx) + offset, (u32)sizeInBytes, fmt);
	}

	virtual void GFXDrawInstanced(u32 VertexCountPerInstance, u32 InstanceCount, u32 StartVertexLocation, u32 StartInstanceLocation) override
	{
		cmdListBundleApi.DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
	}

	virtual void GFXDrawIndexedInstanced(u32 IndexCountPerInstance, u32 InstanceCount, u32 StartIndexLocation, i32 BaseVertexLocation, u32 StartInstanceLocation) override
	{
		cmdListBundleApi.DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}
	
	virtual void GFXExecuteBundle(usize cmdBundleIdx) override
	{
		cmdListDirectApi.ExecuteBundle(m_impl->resCommandBundles[cmdBundleIdx]->GetCommandListIndex());
	}
	

	virtual void CMPSetProgram(usize gfxProgramIdx) override
	{
		gfx_dx12_program* program = m_impl->resGfxPrograms[gfxProgramIdx].get();

		cmdListBundleApi.SetComputeRootSignature(program->rootSignatureIndex);
	}

	virtual void CMPSetProgramState(usize gfxProgramIdx, usize programStateIdx) override
	{
		gfx_dx12_program* program = m_impl->resGfxPrograms[gfxProgramIdx].get();
		cmdListBundleApi.SetPipelineState(program->graphicsPipelineStateIndices[programStateIdx]);
	}

	virtual void CMPSetDescriptorTableCbvSrvUav(i32 slot, usize blockIndex) override
	{
		cmdListBundleApi.SetComputeRootDescriptorTable(slot, activeHeap->GetGPUOffsetCBVSRVUAV(blockIndex));
	}

	virtual void CMPSetDescriptorTableSamplers(i32 slot, usize blockIndex) override
	{
		cmdListBundleApi.SetComputeRootDescriptorTable(slot, activeHeap->GetGPUOffsetSamplers(blockIndex));
	}

	virtual void CMPSetRootParameterCBV(i32 slot, usize cbv, u64 offset) override
	{
		cmdListBundleApi.SetComputeRootConstantBufferView(slot, m_impl->gpuCtx.ResourceGetGPUAddress(cbv) + offset);
	}

	virtual void CMPSetRootParameterUAV(i32 slot, usize uav, u64 offset) override
	{
		cmdListBundleApi.SetComputeRootUnorderedAccessView(slot, m_impl->gpuCtx.ResourceGetGPUAddress(uav) + offset);
	}

	virtual void CMPSetRootParameterSRV(i32 slot, usize srv, u64 offset) override
	{
		cmdListBundleApi.SetComputeRootShaderResourceView(slot, m_impl->gpuCtx.ResourceGetGPUAddress(srv) + offset);
	}

	virtual void CMPSetRootParameterConstants(i32 slot, const void* data, u32 sizeInDwords, u32 offsetInDwords = 0) override
	{
		cmdListBundleApi.SetComputeRoot32BitConstants(slot, sizeInDwords, data, offsetInDwords);
	}

	virtual void CMPDispatch(u32 ThreadGroupCountX, u32 ThreadGroupCountY, u32 ThreadGroupCountZ) override
	{
		cmdListBundleApi.Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
	}

	// Direct command list specific API calls
	// -----------------------------------

	virtual void GFXSetViewports(const cfc::gpu_viewport* viewports, usize numViewports) override
	{
		cmdListDirectApi.RSSetViewports((u32)numViewports, viewports);
	}

	virtual void GFXSetScissorRects(const cfc::gpu_rectangle* scissorRects, usize numScissorRects) override
	{
		cmdListDirectApi.RSSetScissorRects((u32)numScissorRects, scissorRects);
	}

	virtual void GFXSetRenderTargets(const usize* rtvOffsets, usize numRtvs, usize dsvOffset) override
	{
		cmdListDirectApi.OMSetRenderTargets((u32)numRtvs, rtvOffsets, false, dsvOffset);
	}

	virtual void GFXClearRenderTarget(usize rtvOffset, const float* clearColor = nullptr) override
	{
		cmdListDirectApi.ClearRenderTargetView(rtvOffset, clearColor, 0, nullptr);
	}

	virtual void GFXClearDepthStencilTarget(usize dsvOffset, float clearDepth = 1.0f, u8 clearStencil = 0) override
	{
		cmdListDirectApi.ClearDepthStencilView(dsvOffset, true, true, clearDepth, clearStencil, 0, nullptr);
	}

	virtual void ExecuteBarrier(cfc::gpu_resourcebarrier_desc* barrierList, usize count) override
	{
		cmdListDirectApi.ResourceBarrier((u32)count, barrierList);
	}

	virtual u32 InsertTimerQuery() override 
	{
		const u32 timerQueryIndex = m_impl->gpuTimerQueryIndex.fetch_add(1) % MAX_NUM_GPU_TIMER_QUERIES;
		cmdListDirectApi.EndQuery(m_impl->gpuTimerQueryHeap, cfc::gpu_query_type::Timestamp, timerQueryIndex);
		return timerQueryIndex;
	}

	virtual void ResolveQueryData(usize queryHeapIdx, cfc::gpu_query_type type, u32 startIndex, u32 numQueries, usize destinationResourceIdx, u64 alignedDestinationBufferOffset) override
	{
		cmdListBundleApi.ResolveQueryData(queryHeapIdx, type, startIndex, numQueries, destinationResourceIdx, alignedDestinationBufferOffset);
	}
	// -----------------------------------

	usize cmdListIdx;
	usize cmdAllocatorIdx;
	gpu_dx12_cmdlist_direct_api cmdListDirectApi;
	gpu_dx12_cmdlist_bundle_api cmdListBundleApi;
	gfx_dx12_desc_heap* activeHeap = nullptr;

	_imp_dx12_gfx* m_impl;
	usize m_index;
	type m_type;
};

class dx12_gfx_render_target
{
public:
	dx12_gfx_render_target(_imp_dx12_gfx* impl)
	{
		m_impl = impl;
	}

	virtual ~dx12_gfx_render_target()
	{
		m_impl->parent->QueueDestroy(gpu_object_type::Resource, m_resourceID);
	}

	void Init(i32 width, i32 height, cfc::gpu_format_type format, const cfc::gpu_defaultclear_desc& defaultClear)
	{
		m_format = format;

		cfc::gpu_defaultclear_desc clear = defaultClear.Format == gpu_format_type::Unknown ? gpu_defaultclear_desc(format) : defaultClear;

		m_width = width;
		m_height = height;

		stl_assert(m_index < g_maxRenderTargets);

		bool isDepthRT = gpu_format_type_query::IsDepthType(format);
		if (isDepthRT)
		{
			m_dsvOffset = m_impl->gpuCtx.DescriptorHeapGetCPUAddressStart(m_impl->renderTargetDSVHeap) + m_index * m_impl->dsvDescriptorSize;
			m_resourceID = m_impl->gpuCtx.CreateCommittedResource(gpu_heap_type::Default, gpu_resource_desc::Tex2D(format, width, height, 1, 0, cfc::gpu_resource_desc::resourceflags::AllowDepthStencil), gpu_resourcestate::DepthWrite, &clear);
			stl_assert(m_impl->gpuCtx.CreateDescriptorDSVTexture(m_resourceID, m_dsvOffset));
		}
		else
		{
			m_rtvOffset = m_impl->gpuCtx.DescriptorHeapGetCPUAddressStart(m_impl->renderTargetRTVHeap) + m_index * m_impl->rtvDescriptorSize;
			m_resourceID = m_impl->gpuCtx.CreateCommittedResource(gpu_heap_type::Default, gpu_resource_desc::Tex2D(format, width, height, 1, 1, cfc::gpu_resource_desc::resourceflags::AllowRenderTarget), gpu_resourcestate::RenderTarget, &clear);
			stl_assert(m_impl->gpuCtx.CreateDescriptorRTVTexture(m_resourceID, m_rtvOffset, format));
		}
	}

	_imp_dx12_gfx* m_impl = nullptr;
	usize m_index = cfc::invalid_index;
	usize m_resourceID = cfc::invalid_index;
	usize m_rtvOffset = cfc::invalid_index;
	usize m_dsvOffset = cfc::invalid_index;
	i32 m_width = 0;
	i32 m_height = 0;
	cfc::gpu_format_type m_format;
};


gfx_dx12::gfx_dx12()
{

}

gfx_dx12::~gfx_dx12()
{
	WaitForGpu();

	// force no fullscreen on exit
	m_impl->gpuCtx.SwapchainSetFullscreenState(m_impl->swapChain, false);

	// destroy all remaining resources
	for (auto& frameDestroy : m_impl->deletionQueue)
	{
		for (auto& destroyPair : frameDestroy.second)
			m_impl->gpuCtx.Destroy(destroyPair.first, destroyPair.second);
	}
	m_impl->deletionQueue.clear();

#if MICROPROFILE_ENABLED == 1
	// destroy Microprofiler
	MicroProfileGpuShutdown();
#endif
	m_impl.destroy<>();
}

void gfx_dx12::Init(cfc::context& ctx, usize deviceID /*= 0*/, u32 numFrames /*= 2*/, gpu_swapimage_type imgType /*= gpu_swapimage_type::Rgba8Unorm*/, gpu_format_type depthType /*= gpu_format_type::D32Float*/, gpu_swapflip_type flipType /*= gpu_swapflip_type::Discard*/)
{
	gfx::Init(ctx, deviceID, numFrames, imgType, depthType, flipType);
	m_impl.init<>();

	m_impl->parent = this;
	m_impl->ctx = &ctx;
	m_impl->numFrames = numFrames;
	m_impl->resFrameDepth_type = depthType;

	// hook window resize
	m_impl->evResize = [this](cfc::window::eventData ev) { m_impl->resized = true; };
	m_impl->ctx->Window->OnResize += m_impl->evResize;

	// initialize device
	m_impl->gpuCtx.DeviceInit(deviceID);

	// check for MGPU
	if (m_impl->gpuCtx.DeviceIsMGPUEnabled())
	{
		m_impl->numFramesPerNode = numFrames / m_impl->gpuCtx.DeviceGetNodeCount();
		stl_assert(m_impl->numFramesPerNode * m_impl->gpuCtx.DeviceGetNodeCount() == m_impl->numFrames);
	}
	else
	{
		m_impl->numFramesPerNode = numFrames;
	}


	// initialize command queue
	m_impl->cmdQueue = m_impl->gpuCtx.CreateCommandQueue(gpu_commandlist_type::Direct);

#if MICROPROFILE_ENABLED == 1
	// init Microprofiler
	//MICROPROFILE_GPU_INIT_QUEUE("DX12-GPU");
	MicroProfileGpuInitD3D12(m_impl->gpuCtx.DX12_GetDevice(), m_impl->gpuCtx.DX12_GetCommandQueue(m_impl->cmdQueue));
#endif

	// init swap chain targets
	for (u32 i = 0; i < stl_math_max(2, m_impl->numFrames); i++)
		m_impl->resFrameRT[i] = cfc::invalid_index;

	// initialize swap chain
	m_impl->swapChain = m_impl->gpuCtx.CreateSwapChain(m_impl->cmdQueue, ctx.Window->GetWidth(), ctx.Window->GetHeight(), ctx.Window->GetPlatformSpecificHandle(), true, imgType, flipType, stl_math_max(2, m_impl->numFrames), 1, 0);
	m_impl->isFullscreen = DX12_GetContext()->SwapchainGetFullscreenState(m_impl->swapChain);
	m_impl->resized = false;

	// set rt type
	if (imgType == gpu_swapimage_type::Rgba8Unorm)				m_impl->resFrameRT_type = gpu_format_type::Rgba8Unorm;
	else if (imgType == gpu_swapimage_type::Rgba8UnormSrgb)		m_impl->resFrameRT_type = gpu_format_type::Rgba8UnormSrgb;
	else if (imgType == gpu_swapimage_type::Rgba16FloatSrgb)	m_impl->resFrameRT_type = gpu_format_type::Rgba16Float;
	else if (imgType == gpu_swapimage_type::Rgb10A2Unorm)		m_impl->resFrameRT_type = gpu_format_type::Rgb10A2Unorm;

	// initialize fence
	m_impl->fncFrame = m_impl->gpuCtx.CreateFence(0, gpu_fenceshare_type::Unshared);
	m_impl->fnevFrame = m_impl->gpuCtx.CreateFenceEvent();
	m_impl->fncWaitForGpu = m_impl->gpuCtx.CreateFence(0, gpu_fenceshare_type::Unshared);
	m_impl->fnevWaitForGpu = m_impl->gpuCtx.CreateFenceEvent();

	// init rtv/dsv heap
	m_impl->backbufferRtvHeap = m_impl->gpuCtx.CreateDescriptorHeap(gpu_descriptorheap_type::Rtv, numFrames, false);
	if (m_impl->resFrameDepth_type != gpu_format_type::Unknown)
		m_impl->backbufferDsvHeap = m_impl->gpuCtx.CreateDescriptorHeap(gpu_descriptorheap_type::Dsv, 1, false);

	m_impl->renderTargetRTVHeap = m_impl->gpuCtx.CreateDescriptorHeap(gpu_descriptorheap_type::Rtv, g_maxRenderTargets, false);
	m_impl->renderTargetDSVHeap = m_impl->gpuCtx.CreateDescriptorHeap(gpu_descriptorheap_type::Dsv, g_maxRenderTargets, false);
	
	// init frame rt counters
	for (u32 i = 0; i < m_impl->numFrames; i++)
		m_impl->resFrameRT[i] = cfc::invalid_index;

	// init increment
	m_impl->rtvDescriptorSize = m_impl->gpuCtx.GetDescriptorIncrementSize(gpu_descriptorheap_type::Rtv);
	m_impl->dsvDescriptorSize = m_impl->gpuCtx.GetDescriptorIncrementSize(gpu_descriptorheap_type::Dsv);

	// call resize (creating render/depth buffer(s))
	_Resize();

	// setup gpu timer query read back buffer
	m_impl->gpuCtx.SetStablePowerState(true);
	m_impl->gpuTimerQueryTimeStampFrequency = m_impl->gpuCtx.GetTimeStampFrequency(m_impl->cmdQueue);
	m_impl->gpuCtx.SetStablePowerState(false);
	m_impl->gpuTimerQueryIndex.store(0);
	m_impl->gpuTimerQueryFrameCntr = 0;
	m_impl->gpuTimerQueryFnc = m_impl->gpuCtx.CreateFence(0, gpu_fenceshare_type::Unshared);
	m_impl->gpuTimerQueryFnev = m_impl->gpuCtx.CreateFenceEvent();
	m_impl->gpuTimerQueryResults.resize(MAX_NUM_GPU_TIMER_QUERIES);
	gfx_resource_stream* resourceStream = GetResourceStream(AddResourceStream());
	m_impl->gpuTimerQueryReadbackBuffer = resourceStream->AddDynamicResource(gfx_resource_type::CopyDest, MAX_NUM_GPU_TIMER_QUERIES * sizeof(u64), false, true);
	resourceStream->Flush();
	resourceStream->WaitForFinish();
	RemoveResourceStream(resourceStream->GetIndex());

	m_impl->gpuTimerQueryHeap = m_impl->gpuCtx.CreateQueryHeap(gpu_queryheap_type::TimeStamp, MAX_NUM_GPU_TIMER_QUERIES);

	for (u32 i = 0; i < GetTimerQueryFrameDelayQuantity(); i++)
	{
		m_impl->gpuTimerQueryResolveCommandLists[i] = AddCommandList();
		memset(&m_impl->gpuTimerQueryFrameInfo[i], 0, sizeof(query_timer_frame));
	}
}

void gfx_dx12::_Resize()
{
	MICROPROFILE_SCOPEI("DX12", "Resize", 0);

	// calculate swapchain buffers
	u32 swapChainFrames = stl_math_max(2, m_impl->numFrames);

	// check for minimization
	if (m_impl->ctx->Window->GetWidth() == 0 || m_impl->ctx->Window->GetHeight() == 0)
		return;

	// update gpu_viewport & scissor
	gpu_viewport _viewport(0, 0, m_impl->ctx->Window->GetWidthAsFloat(), m_impl->ctx->Window->GetHeightAsFloat());
	gpu_rectangle _scissorRect(0, 0, m_impl->ctx->Window->GetWidth(), m_impl->ctx->Window->GetHeight());
	m_impl->gpu_viewport = _viewport;
	m_impl->scissorRect = _scissorRect;


	// destroy previous buffers
	for (u32 i = 0; i < swapChainFrames; i++)
	{
		if (m_impl->resFrameRT[i] != cfc::invalid_index)
			m_impl->gpuCtx.Destroy(gpu_object_type::Resource, m_impl->resFrameRT[i]);
	}

	if (m_impl->resFrameDepth != cfc::invalid_index)
		m_impl->gpuCtx.Destroy(gpu_object_type::Resource, m_impl->resFrameDepth);

	// resize swap chain
	stl_assert(m_impl->gpuCtx.SwapchainResizeBuffer(m_impl->swapChain, (u32)m_impl->gpu_viewport.Width, (u32)m_impl->gpu_viewport.Height, swapChainFrames));

	// reset the frame index to the current back buffer index.
	m_impl->frameIndex = m_impl->gpuCtx.SwapchainGetCurrentBackbufferIndex(m_impl->swapChain);

	// update swap chain & rtvs
	usize rtvHeapStart = m_impl->gpuCtx.DescriptorHeapGetCPUAddressStart(m_impl->backbufferRtvHeap);
	for (u32 i = 0; i < swapChainFrames; i++)
	{
		m_impl->resFrameRT[i] = m_impl->gpuCtx.CreateSwapChainResource(m_impl->swapChain, i);
		stl_assert(m_impl->gpuCtx.CreateDescriptorRTVTexture(m_impl->resFrameRT[i], rtvHeapStart + m_impl->rtvDescriptorSize * i, m_impl->resFrameRT_type));
		m_impl->gpuCtx.ResourceSetName(GetBackbufferRTResource(i), stl_string_advanced::sprintf("BackbufferRT-%d", i).c_str());
	}

	if(m_impl->resFrameDepth_type != gpu_format_type::Unknown)
	{
		// create depth buffer & dsv
		gpu_defaultclear_desc dclear(m_impl->resFrameDepth_type);
		m_impl->resFrameDepth = m_impl->gpuCtx.CreateCommittedResource(gpu_heap_type::Default, gpu_resource_desc::Tex2D(m_impl->resFrameDepth_type, (u64)m_impl->gpu_viewport.Width, (u32)m_impl->gpu_viewport.Height, 1, 1, gpu_resource_desc::resourceflags::AllowDepthStencil), gpu_resourcestate::DepthWrite, &dclear);
		stl_assert(m_impl->gpuCtx.CreateDescriptorDSVTexture(m_impl->resFrameDepth, m_impl->gpuCtx.DescriptorHeapGetCPUAddressStart(m_impl->backbufferDsvHeap)));

		m_impl->gpuCtx.ResourceSetName(m_impl->resFrameDepth, "m_impl->resFrameDepth");
	}

	// reset counters
	for (u32 i = 0; i < m_impl->numFrames; i++)
		m_impl->frameFenceCtr[i] = 1;
}


gpu_features cfc::gfx_dx12::GetDeviceFeatures() const
{
	return m_impl->gpuCtx.DeviceGetFeatures();
}

usize cfc::gfx_dx12::AddShaderFromMemory(const void* fileData, usize fileDataSize, const char* funcName /*= "main"*/, const char* shaderType /*= "vs_5_0"*/, const char* shaderFilename /*= "unknown.shd"*/, const char* defineData /*=null*/)
{
	MICROPROFILE_SCOPEI("DX12", "AddShaderFromMemory", 0);

	// copy defines into the shader
	char copyBuffer[g_shaderDefineCopyBufferSize];
	usize shaderDataSize = fileDataSize;
	if (defineData != nullptr)
	{
		usize shaderDefinesSize = strlen(defineData);
		
		// make sure that our copy buffer is big enough to copy the shader data and add the defines
		stl_assert(shaderDefinesSize + fileDataSize < g_shaderDefineCopyBufferSize);
		
		strcpy(copyBuffer, defineData);
		strcpy(copyBuffer + shaderDefinesSize, (char*)fileData);
		shaderDataSize = shaderDefinesSize + fileDataSize;
	}

	const void* shaderData = defineData != nullptr ? copyBuffer : fileData;
	
	usize shaderIdx;
	stl_string shrErrors;
	shaderIdx = m_impl->gpuCtx.CreateShaderBlobCompile(shaderData, shaderDataSize, funcName, shaderType, shaderFilename, &shrErrors);
	if (shaderIdx == cfc::invalid_index)
	{
		m_impl->ctx->Log->Logf(cfc::logflags::ScpEngine | cfc::logflags::SevError, "Shader compilation failure: %s", shrErrors.c_str());
	}

	return shaderIdx;
}

usize cfc::gfx_dx12::AddDescriptorHeap(i32 maxCbvSrvUav, i32 maxSamplers)
{
	MICROPROFILE_SCOPEI("DX12", "AddDescriptorHeap", 0);

	stl_unique_ptr<gfx_dx12_desc_heap> ret(new gfx_dx12_desc_heap(m_impl.get()));

	if (maxCbvSrvUav > 0)
	{
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapCbvSrvUav].heap = m_impl->gpuCtx.CreateDescriptorHeap(gpu_descriptorheap_type::CbvSrvUav, maxCbvSrvUav, true, 0);
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapCbvSrvUav].size = maxCbvSrvUav;
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapCbvSrvUav].descriptorSize = m_impl->gpuCtx.GetDescriptorIncrementSize(gpu_descriptorheap_type::CbvSrvUav);
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapCbvSrvUav].cpuStart = m_impl->gpuCtx.DescriptorHeapGetCPUAddressStart(ret->m_heapInfo[gfx_dx12_desc_heap::HeapCbvSrvUav].heap);
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapCbvSrvUav].gpuStart = m_impl->gpuCtx.DescriptorHeapGetGPUAddressStart(ret->m_heapInfo[gfx_dx12_desc_heap::HeapCbvSrvUav].heap);
		ret->heaps[ret->numHeaps++] = ret->m_heapInfo[gfx_dx12_desc_heap::HeapCbvSrvUav].heap;

		// special CPU UAV heap for clear
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapUavCpu].heap = m_impl->gpuCtx.CreateDescriptorHeap(gpu_descriptorheap_type::CbvSrvUav, maxCbvSrvUav, false, 0);
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapUavCpu].size = maxCbvSrvUav;
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapUavCpu].descriptorSize = m_impl->gpuCtx.GetDescriptorIncrementSize(gpu_descriptorheap_type::CbvSrvUav);
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapUavCpu].cpuStart = m_impl->gpuCtx.DescriptorHeapGetCPUAddressStart(ret->m_heapInfo[gfx_dx12_desc_heap::HeapUavCpu].heap);
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapUavCpu].gpuStart = m_impl->gpuCtx.DescriptorHeapGetGPUAddressStart(ret->m_heapInfo[gfx_dx12_desc_heap::HeapUavCpu].heap);
	}
	if (maxSamplers > 0)
	{
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapSamplers].heap = m_impl->gpuCtx.CreateDescriptorHeap(gpu_descriptorheap_type::Samplers, maxSamplers, true, 0);
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapSamplers].size = maxSamplers;
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapSamplers].descriptorSize = m_impl->gpuCtx.GetDescriptorIncrementSize(gpu_descriptorheap_type::Samplers);
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapSamplers].cpuStart = m_impl->gpuCtx.DescriptorHeapGetCPUAddressStart(ret->m_heapInfo[gfx_dx12_desc_heap::HeapSamplers].heap);
		ret->m_heapInfo[gfx_dx12_desc_heap::HeapSamplers].gpuStart = m_impl->gpuCtx.DescriptorHeapGetGPUAddressStart(ret->m_heapInfo[gfx_dx12_desc_heap::HeapSamplers].heap);
		ret->heaps[ret->numHeaps++] = ret->m_heapInfo[gfx_dx12_desc_heap::HeapSamplers].heap;
	}

	usize idx = m_impl->resDescriptorHeaps.insert();
	ret->m_index = idx;
	m_impl->resDescriptorHeaps[idx].swap(ret);

	return idx;
}

usize cfc::gfx_dx12::_AddProgram(usize vertexShader, usize pixelShader, usize geometryShader, usize hullShader, usize domainShader, usize computeShader)
{
	MICROPROFILE_SCOPEI("DX12", "GfxStateCompile", 0);

	stl_unique_ptr<gfx_dx12_program> program(new gfx_dx12_program(m_impl.get()));

	// store shaders in program
	usize shrIndices[] = { vertexShader, pixelShader, geometryShader, hullShader, domainShader, computeShader };
	memcpy(&program->shaderIndices[0], shrIndices, sizeof(shrIndices));

	// reflect shaders
	int numShaders = 0;
	const cfc::gpu_shaderreflection_desc* shrList[(usize)gfx_shader_type::COUNT];
	cfc::gpu_shadervisibility_type shrVisibility[(usize)gfx_shader_type::COUNT];
	const cfc::gpu_shaderreflection_desc* reflected[6];
	for (usize i = 0; i < sizeof(shrIndices) / sizeof(shrIndices[0]); i++)
	{
		if (shrIndices[i] != cfc::invalid_index)
		{
			reflected[i] = &m_impl->gpuCtx.ShaderBlobGetReflection(shrIndices[i]);

			// get bind point for resource
			const stl_vector<cfc::gpu_shaderreflection_desc::gpu_resource_desc>& resources = reflected[i]->Resources;
			for (u32 j = 0; j < resources.size(); ++j)
				program->shaderBindings.insert(std::pair<stl_string, usize>(resources[j].Name, resources[j].BindPoint));

			shrList[numShaders] = reflected[i];
			switch (i)
			{
			case (int)gfx_shader_type::Vertex:
			{
				// get bind point for VS input
				shrVisibility[numShaders++] = cfc::gpu_shadervisibility_type::Vertex;
				const stl_vector<cfc::gpu_shaderreflection_desc::parameter_desc>& vsInputs = reflected[i]->Inputs;
				for (u32 j = 0; j < vsInputs.size(); ++j)
					program->shaderBindings.insert(std::pair<stl_string, usize>(vsInputs[j].SemanticName, vsInputs[j].SemanticIndex));
				break;
			}
			case (int)gfx_shader_type::Pixel:		shrVisibility[numShaders++] = cfc::gpu_shadervisibility_type::Pixel;		break;
			case (int)gfx_shader_type::Geometry:	shrVisibility[numShaders++] = cfc::gpu_shadervisibility_type::Geometry;	break;
			case (int)gfx_shader_type::Hull:		shrVisibility[numShaders++] = cfc::gpu_shadervisibility_type::Hull;		break;
			case (int)gfx_shader_type::Domain:		shrVisibility[numShaders++] = cfc::gpu_shadervisibility_type::Domain;		break;
			case (int)gfx_shader_type::Compute:		shrVisibility[numShaders++] = cfc::gpu_shadervisibility_type::Compute;		break;
			default:								stl_assert(false);															break; // unsupported 
			}
		}
	}

	// build root signature
	m_impl->parent->_ExtractRootSignatureFromShaders(shrList, shrVisibility, numShaders, program->rootSignatureDescriptor);
	program->rootSignatureIndex = m_impl->gpuCtx.CreateRootSignature(program->rootSignatureDescriptor);

	// insert into resource list
	usize idx = m_impl->resGfxPrograms.insert();
	program->m_index = idx;
	m_impl->resGfxPrograms[idx].swap(program);
	return idx;
}

usize cfc::gfx_dx12::AddGraphicsProgram(usize vertexShader /*= cfc::invalid_index*/, usize pixelShader /*= cfc::invalid_index*/, usize geometryShader /*= cfc::invalid_index*/, usize hullShader /*= cfc::invalid_index*/, usize domainShader /*= cfc::invalid_index*/)
{
	return _AddProgram(vertexShader, pixelShader, geometryShader, hullShader, domainShader, cfc::invalid_index);
}

usize cfc::gfx_dx12::AddGraphicsProgramPipelineState(usize graphicsProgramIdx, const gfx_gfxprogram_desc& descriptor)
{
	gfx_dx12_program* program = m_impl->resGfxPrograms[graphicsProgramIdx].get();

	// copy & update pipeline state descriptor
	cfc::gpu_graphicspipelinestate_desc pipelineDesc(descriptor.Pipeline);
	pipelineDesc.RootSignatureIdx = program->rootSignatureIndex;
	pipelineDesc.VertexShaderIdx = program->shaderIndices[(int)gfx_shader_type::Vertex];
	pipelineDesc.PixelShaderIdx = program->shaderIndices[(int)gfx_shader_type::Pixel];
	pipelineDesc.GeometryShaderIdx = program->shaderIndices[(int)gfx_shader_type::Geometry];
	pipelineDesc.DomainShaderIdx = program->shaderIndices[(int)gfx_shader_type::Domain];
	pipelineDesc.HullShaderIdx = program->shaderIndices[(int)gfx_shader_type::Hull];

	// generate input layout if requested
	if (descriptor.GenerateInputLayout && pipelineDesc.VertexShaderIdx != cfc::invalid_index)
		m_impl->parent->_ExtractInputLayoutFromShader(m_impl->gpuCtx.ShaderBlobGetReflection(pipelineDesc.VertexShaderIdx), pipelineDesc);

	// QQQ NON THREAD SAFE!
	program->graphicsPipelineStateIndices.push_back(m_impl->gpuCtx.CreatePipelineState(pipelineDesc));
	return program->graphicsPipelineStateIndices.size() - 1;
}

usize cfc::gfx_dx12::AddComputeProgram(usize computeShader /*= cfc::invalid_index*/)
{
	return _AddProgram(cfc::invalid_index, cfc::invalid_index, cfc::invalid_index, cfc::invalid_index, cfc::invalid_index, computeShader);
}

usize cfc::gfx_dx12::AddComputeProgramPipelineState(usize computeProgramIdx, const gfx_cmpprogram_desc& descriptor)
{
	gfx_dx12_program* program = m_impl->resGfxPrograms[computeProgramIdx].get();

	// copy & update pipeline state descriptor
	cfc::gpu_computepipelinestate_desc pipelineDesc(descriptor.Pipeline);
	pipelineDesc.RootSignatureIdx = program->rootSignatureIndex;
	pipelineDesc.ComputeShaderIdx = program->shaderIndices[(int)gfx_shader_type::Compute];

	// QQQ NON THREAD SAFE!
	program->graphicsPipelineStateIndices.push_back(m_impl->gpuCtx.CreatePipelineState(pipelineDesc));
	return program->graphicsPipelineStateIndices.size() - 1;
}

usize cfc::gfx_dx12::AddCommandList()
{
	stl_unique_ptr<gfx_dx12_command_list> obj(new gfx_dx12_command_list(m_impl.get(), gfx_command_list::type::Direct, 0));
	usize idx = m_impl->resCommandLists.insert();
	obj->m_index = idx;
	m_impl->resCommandLists[idx].swap(obj);
	return idx;
}

usize cfc::gfx_dx12::AddCommandBundle(usize cmdAllocatorBundleId)
{
	stl_unique_ptr<gfx_dx12_command_list> obj(new gfx_dx12_command_list(m_impl.get(), gfx_command_list::type::Bundle, cmdAllocatorBundleId));
	usize idx = m_impl->resCommandBundles.insert();
	obj->m_index = idx;
	m_impl->resCommandBundles[idx].swap(obj);
	return idx;
}

usize cfc::gfx_dx12::AddBundleAllocator()
{
	usize cmdAllocator = m_impl->gpuCtx.CreateCommandAllocator(gpu_commandlist_type::Bundle);
	usize idx = m_impl->resBundleCommandAllocator.insert();
	m_impl->resBundleCommandAllocator[idx] = cmdAllocator;
	return idx;
}

usize cfc::gfx_dx12::AddResourceStream()
{
	MICROPROFILE_SCOPEI("DX12", "AddResourceStream", 0);

	stl_unique_ptr<gfx_dx12_resource_stream> obj(new gfx_dx12_resource_stream(m_impl.get()));
	usize idx = m_impl->resResourceStreams.insert();
	obj->m_index = idx;
	m_impl->resResourceStreams[idx].swap(obj);
	return idx;
}

usize cfc::gfx_dx12::AddRenderTarget2D(i32 width, i32 height, cfc::gpu_format_type format, cfc::gpu_defaultclear_desc defaultClear /*= cfc::gpu_defaultclear_desc()*/)
{
	stl_unique_ptr<dx12_gfx_render_target> obj(new dx12_gfx_render_target(m_impl.get()));
	usize idx = m_impl->resRenderTargets.insert();
	obj->m_index = idx;
	obj->Init(width, height, format, defaultClear);
	m_impl->resRenderTargets[idx].swap(obj);
	return idx;
}

void cfc::gfx_dx12::RemoveResource(usize bufferID)
{
	MICROPROFILE_SCOPEI("DX12", "RemoveResource", 0);
	m_impl->parent->QueueDestroy(gpu_object_type::Resource, bufferID);
}

void cfc::gfx_dx12::RemoveShader(usize shaderID)
{
	MICROPROFILE_SCOPEI("DX12", "RemoveShader", 0);
	m_impl->parent->QueueDestroy(gpu_object_type::ShaderBlob, shaderID);
}

void cfc::gfx_dx12::RemoveGraphicsProgram(usize gfxProgramID)
{
	m_impl->resGfxPrograms.erase(gfxProgramID);
}

void cfc::gfx_dx12::RemoveComputeProgram(usize cmpProgramIdx)
{
	m_impl->resGfxPrograms.erase(cmpProgramIdx);
}

void cfc::gfx_dx12::RemoveDescriptorHeap(usize idx)
{
	m_impl->resDescriptorHeaps.erase(idx);
}

void cfc::gfx_dx12::RemoveCommandList(usize idx)
{
	m_impl->resCommandLists.erase(idx);
}

void cfc::gfx_dx12::RemoveCommandBundle(usize idx)
{
	m_impl->resCommandBundles.erase(idx);
}

void cfc::gfx_dx12::RemoveBundleAllocator(usize idx)
{
	m_impl->resBundleCommandAllocator.erase(idx);
}

void cfc::gfx_dx12::RemoveResourceStream(usize idx)
{
	m_impl->resResourceStreams.erase(idx);
}

void cfc::gfx_dx12::RemoveRenderTarget(usize rtID)
{
	m_impl->resRenderTargets.erase(rtID);
}

void gfx_dx12::ExecuteCommandLists(const usize* commandLists, usize numCommandLists) 
{
	usize* listIndices = (usize*)_alloca(numCommandLists * sizeof(usize));
	for (usize i = 0; i < numCommandLists; i++)
	{
		auto& cmdlist = m_impl->resCommandLists[commandLists[i]];
		listIndices[i] = cmdlist->GetCommandListIndex();
	}
	m_impl->gpuCtx.CommandQueueExecuteCommandLists(m_impl->cmdQueue, (u32)numCommandLists, listIndices);
}

void gfx_dx12::WaitForGpu()
{
	MICROPROFILE_SCOPEI("DX12", "WaitForGPU", 0);
	m_impl->gpuCtx.CommandQueueSignal(m_impl->cmdQueue, m_impl->fncWaitForGpu, m_impl->fnctrWaitForGpu);				// add a fence after completion of current tasks in queue
	m_impl->gpuCtx.FenceSetEventOnCompletion(m_impl->fncWaitForGpu, m_impl->fnevWaitForGpu, m_impl->fnctrWaitForGpu++);	// set fence to signal fenceEvent when marker has been reached
	m_impl->gpuCtx.FenceEventWaitFor(m_impl->fnevWaitForGpu);															// wait for event to be signaled
}

bool gfx_dx12::Present(u32 swapInterval /*= 1*/, u32 flags /*= 0*/)
{	
	// check for swapchain invalidations
	if (m_impl->resized || m_impl->isFullscreen != DX12_GetContext()->SwapchainGetFullscreenState(m_impl->swapChain))
	{
		m_impl->isFullscreen = DX12_GetContext()->SwapchainGetFullscreenState(m_impl->swapChain);
		m_impl->resized = false;

		WaitForGpu();
		_Resize();
		return true; // force a new frame render
	}

	bool ret = false;
	{
		MICROPROFILE_SCOPEI("DX12", "Present", 0);
		ret = m_impl->gpuCtx.SwapchainPresent(m_impl->swapChain, swapInterval, flags);
	}

	if (m_impl->gpuCtx.DeviceIsMGPUEnabled())
	{
		// move to next frame (multi GPU)
		m_impl->gpuCtx.MGPUSwitchToNextNode();
		m_impl->nodeIndex = m_impl->gpuCtx.MGPUGetCurrentNodeID();
		
		MICROPROFILE_SCOPEI("DX12", "PresentNextFrame", 0);
		u64 lastFenceValue = m_impl->frameFenceCtr[m_impl->frameIndex];
		m_impl->gpuCtx.CommandQueueSignal(m_impl->cmdQueue, m_impl->fncFrame, lastFenceValue);

		if (m_impl->nodeIndex == 0)
			m_impl->frameIndex = (m_impl->frameIndex + 1) % (m_impl->numFramesPerNode);

		if (m_impl->gpuCtx.FenceGetCompletedValue(m_impl->fncFrame) < m_impl->frameFenceCtr[m_impl->frameIndex])
		{
			MICROPROFILE_SCOPEI("DX12", "PresentNextFrameWait", 0);
			m_impl->gpuCtx.FenceSetEventOnCompletion(m_impl->fncFrame, m_impl->fnevFrame, m_impl->frameFenceCtr[m_impl->frameIndex]);
			m_impl->gpuCtx.FenceEventWaitFor(m_impl->fnevFrame);
		}

		m_impl->frameFenceCtr[m_impl->frameIndex] = lastFenceValue + 1;
	}
	else
	{
		// move to next frame (single GPU)
		MICROPROFILE_SCOPEI("DX12", "PresentNextFrame", 0);
		u64 lastFenceValue = m_impl->frameFenceCtr[m_impl->frameIndex];
		m_impl->gpuCtx.CommandQueueSignal(m_impl->cmdQueue, m_impl->fncFrame, lastFenceValue);
		m_impl->frameIndex = m_impl->gpuCtx.SwapchainGetCurrentBackbufferIndex(m_impl->swapChain);
		if (m_impl->gpuCtx.FenceGetCompletedValue(m_impl->fncFrame) < m_impl->frameFenceCtr[m_impl->frameIndex])
		{
			MICROPROFILE_SCOPEI("DX12", "PresentNextFrameWait", 0);
			m_impl->gpuCtx.FenceSetEventOnCompletion(m_impl->fncFrame, m_impl->fnevFrame, m_impl->frameFenceCtr[m_impl->frameIndex]);
			m_impl->gpuCtx.FenceEventWaitFor(m_impl->fnevFrame);
		}

		m_impl->frameFenceCtr[m_impl->frameIndex] = lastFenceValue + 1;
	}

	// flush deletion queue from frame
	m_impl->mtxDeletionQueue.lock();
	if (m_impl->deletionQueue.find(m_impl->frameCounter) != m_impl->deletionQueue.end())
	{
		auto v = std::move(m_impl->deletionQueue[m_impl->frameCounter]);
		m_impl->deletionQueue.erase(m_impl->frameCounter);
		m_impl->mtxDeletionQueue.unlock();

		for (auto& it : v)
		{
			m_impl->gpuCtx.Destroy(it.first, it.second);
		}
	}
	else
		m_impl->mtxDeletionQueue.unlock();

	// increase frame counter
	m_impl->frameCounter++;


	return ret;
}

void cfc::gfx_dx12::ResolveTimerQueries()
{
	// retrieve gpu timer values
	const u32 endIndex = m_impl->gpuTimerQueryIndex.load() % MAX_NUM_GPU_TIMER_QUERIES;

	{
		const query_timer_frame& timerFrame = m_impl->gpuTimerQueryFrameInfo[GetTimerQueryWriteFrameIndex()];

		usize numQueriesInFrame = endIndex - timerFrame.StartIndex;
		if (timerFrame.StartIndex == endIndex)
			return;

		gfx_command_list* currentCmdList = GetCommandList(m_impl->gpuTimerQueryResolveCommandLists[GetTimerQueryWriteFrameIndex()]);
		currentCmdList->Reset();

		// when we loop around, we do an extra query to resolve the values at the beginning of the buffer and resolve the remainder on the back of the buffer
		if (timerFrame.StartIndex > endIndex)
		{
			currentCmdList->ResolveQueryData(m_impl->gpuTimerQueryHeap, gpu_query_type::Timestamp, 0, endIndex, m_impl->gpuTimerQueryReadbackBuffer, 0);
			numQueriesInFrame = MAX_NUM_GPU_TIMER_QUERIES - timerFrame.StartIndex;
		}
		currentCmdList->ResolveQueryData(m_impl->gpuTimerQueryHeap, gpu_query_type::Timestamp, timerFrame.StartIndex, numQueriesInFrame, m_impl->gpuTimerQueryReadbackBuffer, timerFrame.StartIndex * sizeof(u64));

		currentCmdList->Close();
		const usize cmdListIndex = currentCmdList->GetIndex();
		ExecuteCommandLists(&cmdListIndex, 1);
	}

	m_impl->gpuTimerQueryLastResolvedFrame = m_impl->gpuTimerQueryFrameCntr;
	m_impl->gpuCtx.CommandQueueSignal(m_impl->cmdQueue, m_impl->gpuTimerQueryFnc, m_impl->gpuTimerQueryFrameCntr);
	m_impl->gpuTimerQueryFrameCntr++;

	m_impl->gpuTimerQueryFrameInfo[GetTimerQueryWriteFrameIndex()].StartIndex = endIndex;
}

void cfc::gfx_dx12::ReadBackTimerQueryResults()
{
	u64 completed = m_impl->gpuCtx.FenceGetCompletedValue(m_impl->gpuTimerQueryFnc);
	if (completed < m_impl->gpuTimerQueryLastResolvedFrame)
	{
		m_impl->gpuCtx.FenceSetEventOnCompletion(m_impl->gpuTimerQueryFnc, m_impl->gpuTimerQueryFnc, m_impl->gpuTimerQueryLastResolvedFrame);
		m_impl->gpuCtx.FenceEventWaitFor(m_impl->gpuTimerQueryFnev);
	}

	const usize resolveFrameIndex = (m_impl->gpuTimerQueryFrameCntr + (GetTimerQueryFrameDelayQuantity() - 1)) % GetTimerQueryFrameDelayQuantity();
	const usize nextFrameIndex = (resolveFrameIndex + 1) % GetTimerQueryFrameDelayQuantity();
	
	const u32 startQueryIndex = m_impl->gpuTimerQueryFrameInfo[resolveFrameIndex].StartIndex;
	const u32 endQueryIndex = m_impl->gpuTimerQueryFrameInfo[nextFrameIndex].StartIndex;

	usize numQueriesInFrame = endQueryIndex - startQueryIndex;

	if (numQueriesInFrame > 0)
	{
		// when we loop around, we do an extra query to resolve the values at the beginning of the buffer and resolve the remainder on the back of the buffer
		if (startQueryIndex > endQueryIndex)
		{
			char* gpuAddress = (char*)m_impl->gpuCtx.ResourceMap(m_impl->gpuTimerQueryReadbackBuffer, true, 0, 0, endQueryIndex * sizeof(u64));
			memcpy(&m_impl->gpuTimerQueryResults[0], gpuAddress, endQueryIndex * sizeof(u64));
			m_impl->gpuCtx.ResourceUnmap(m_impl->gpuTimerQueryReadbackBuffer, false);

			numQueriesInFrame = MAX_NUM_GPU_TIMER_QUERIES - startQueryIndex;
		}
		char* gpuAddress = (char*)m_impl->gpuCtx.ResourceMap(m_impl->gpuTimerQueryReadbackBuffer, true, 0, startQueryIndex * sizeof(u64), endQueryIndex * sizeof(u64));
		memcpy(&m_impl->gpuTimerQueryResults[startQueryIndex], gpuAddress + startQueryIndex * sizeof(u64), numQueriesInFrame * sizeof(u64));
		m_impl->gpuCtx.ResourceUnmap(m_impl->gpuTimerQueryReadbackBuffer, false);
	}
}

f64 cfc::gfx_dx12::GetTimerQueryResultInMS(const u32 index)
{
	stl_assert(index < m_impl->gpuTimerQueryResults.size());
	return ((f64)(m_impl->gpuTimerQueryResults[index] * 1000) / (f64)m_impl->gpuTimerQueryTimeStampFrequency);
}

f64 cfc::gfx_dx12::GetTimerQueryResultInMS(const gfx_gpu_timer_query& timerQuery)
{
	const f64 startTimerQueryTime = GetTimerQueryResultInMS(timerQuery.GetStartIndex());
	const f64 endTimerQueryTime = GetTimerQueryResultInMS(timerQuery.GetEndIndex());

	//stl_assert(startTimerQueryTime <= endTimerQueryTime);

	return (endTimerQueryTime - startTimerQueryTime);
}

u32 cfc::gfx_dx12::GetGraphicsProgramBindLocation(usize gfxProgramIdx, const char* name)
{
	// TODO: fix and improve!
	stl_unique_ptr<gfx_dx12_program>& program = m_impl->resGfxPrograms[gfxProgramIdx];
	std::map<stl_string, usize>::iterator it = program->shaderBindings.find(name);
	if (it != program->shaderBindings.end())
		return it->second;

	stl_assert(false);

	return (u32)cfc::invalid_index;
}

u32 cfc::gfx_dx12::GetComputeProgramBindLocation(usize cmpProgramIdx, const char* name)
{
	// TODO: fix and improve!
	return GetGraphicsProgramBindLocation(cmpProgramIdx, name);
}

usize cfc::gfx_dx12::GetBackbufferRTResource()
{
	return m_impl->resFrameRT[m_impl->gpuCtx.SwapchainGetCurrentBackbufferIndex(m_impl->swapChain)];
}

usize cfc::gfx_dx12::GetBackbufferRTResource(usize index)
{
	return m_impl->resFrameRT[index];
}

usize cfc::gfx_dx12::GetBackbufferDSResource()
{
	return m_impl->resFrameDepth;
}

usize cfc::gfx_dx12::GetRenderTargetResource(usize rtID)
{
	return m_impl->resRenderTargets[rtID]->m_resourceID;
}

usize cfc::gfx_dx12::GetRenderTargetDSVOffset(usize rtID)
{
	return m_impl->resRenderTargets[rtID]->m_dsvOffset;
}

usize cfc::gfx_dx12::GetRenderTargetRTVOffset(usize rtID)
{
	return m_impl->resRenderTargets[rtID]->m_rtvOffset;
}

i32 cfc::gfx_dx12::GetRenderTargetWidth(usize rtID)
{
	return m_impl->resRenderTargets[rtID]->m_width;
}

i32 cfc::gfx_dx12::GetRenderTargetHeight(usize rtID)
{
	return m_impl->resRenderTargets[rtID]->m_height;
}

cfc::gpu_format_type cfc::gfx_dx12::GetRenderTargetFormat(usize rtID)
{
	return m_impl->resRenderTargets[rtID]->m_format;
}

i32 cfc::gfx_dx12::GetBackbufferWidth()
{
	return m_impl->scissorRect.X2;
}

i32 cfc::gfx_dx12::GetBackbufferHeight()
{
	return m_impl->scissorRect.Y2;
}

usize cfc::gfx_dx12::GetBackbufferRTVOffset()
{
	return m_impl->gpuCtx.DescriptorHeapGetCPUAddressStart(m_impl->backbufferRtvHeap) + GetBackbufferFrameIndex() * m_impl->rtvDescriptorSize;
}

usize cfc::gfx_dx12::GetBackbufferDSVOffset()
{
	return m_impl->gpuCtx.DescriptorHeapGetCPUAddressStart(m_impl->backbufferDsvHeap);
}

gpu_format_type gfx_dx12::GetBackbufferRTVFormat()
{
	return m_impl->resFrameRT_type;
}

gpu_format_type gfx_dx12::GetBackbufferDSVFormat()
{
	return m_impl->resFrameDepth_type;
}

void gfx_dx12::_ExtractRootSignatureFromShaders(const cfc::gpu_shaderreflection_desc** reflectedShaders, cfc::gpu_shadervisibility_type* visibilityTypes, int numShaders, gpu_rootsignature_desc& gfxRootSignature)
{
	struct resourceInfo
	{
		int index;
		stl_vector<stl_pair<usize, usize> > shaders;
	};

	int resourceVarsCount = 0;
	stl_map<stl_string, resourceInfo> resourceVars;
	stl_vector<resourceInfo*> resourceVarsSequential;
	gpu_shaderreflection_desc reflectedResources;
	for (int i = 0; i < numShaders; i++)
	{
		const cfc::gpu_shaderreflection_desc& reflected = *reflectedShaders[i];
		cfc::gpu_shadervisibility_type visType = visibilityTypes[i];

		for (usize j = 0; j < reflected.Resources.size(); j++)
		{
			auto& resource = reflected.Resources[j];
			if (resourceVars.find(resource.Name) == resourceVars.end())
			{
				resourceInfo rinfo;
				rinfo.index = resourceVarsCount++;
				rinfo.shaders.push_back(stl_pair<usize, usize>(i, j));
				resourceVars[resource.Name] = rinfo;
				resourceVarsSequential.push_back(&resourceVars[resource.Name]);
			}
			else
			{
				auto& otherResource = reflectedShaders[resourceVars[resource.Name].shaders[0].first]->Resources[resourceVars[resource.Name].shaders[0].second];
				stl_assert(otherResource.BindPoint == resource.BindPoint); // bindpoint must match if names match
				resourceVars[resource.Name].shaders.push_back(stl_pair<usize, usize>(i, j));
			}
		}
	}

	bool needsCbvSrvUavDescriptorTable = false;
	bool needsSamplerDescriptorTable = false;
	stl_vector<gpu_rootsignature_desc::parameter_range> cbvSrvRanges;
	stl_vector<gpu_rootsignature_desc::parameter_range> samplerRanges;

	for (auto& res : resourceVarsSequential)
	{
		auto& resInfo = *res;
		auto& shr = resInfo.shaders[0];
		auto& resource = reflectedShaders[shr.first]->Resources[shr.second];
		auto vis = resInfo.shaders.size() == 1 ? visibilityTypes[shr.first] : gpu_shadervisibility_type::All;
		switch (resource.Type)
		{
		case gpu_shaderreflection_desc::ConstantBuffer:
			
			if (resource.Name.size() > 2 && resource.Name.substr(0, 2) == "r_")
			{
				// * Parameter directly in Root Signature
				gfxRootSignature.Parameters.push_back(gpu_rootsignature_desc::parameter::CBV(resource.BindPoint, vis, resource.Space));
			}
			else if (resource.Name.size() > 3 && resource.Name.substr(0, 3) == "rc_")
			{
				// * Interpret parameter as constant directly in Root Signature
				auto cbufferSizeInBytes = reflectedShaders[shr.first]->Cbuffers[resource.TypeIndex].SizeInBytes / 4;
				gfxRootSignature.Parameters.push_back(gpu_rootsignature_desc::parameter::Constants(resource.BindPoint, cbufferSizeInBytes, vis, resource.Space));
			}
			else
			{
				// * Parameter in descriptor table
				needsCbvSrvUavDescriptorTable = true;
				cbvSrvRanges.push_back(gpu_rootsignature_desc::parameter_range(gpu_rootsignature_desc::DrtCbv, resource.BindCount, resource.BindPoint, resource.Space));
			}
			break;
		case gpu_shaderreflection_desc::Sampler:
			if (resource.Name.size() > 2 && resource.Name.substr(0, 2) == "r_")
			{
				stl_string resourceTokens[3];
				int elems=stl_string_advanced::split_array(resource.Name, "_", resourceTokens, 3);
				if(elems == 3)
				{
					gpu_samplerfilter_type filterType = gpu_samplerfilter_type::MinMagMipPoint;
					gpu_texaddr_type clampType = gpu_texaddr_type::Clamp;
					u32 maxAnisotropy = 1;

					// filtering mode
					if (resourceTokens[1] == "point") filterType = gpu_samplerfilter_type::MinMagMipPoint;
					else if (resourceTokens[1] == "linear") filterType = gpu_samplerfilter_type::MinMagMipLinear;
					else if (resourceTokens[1] == "trilinear") filterType = gpu_samplerfilter_type::MinMagMipLinear;
					else if (resourceTokens[1] == "bilinear") filterType = gpu_samplerfilter_type::MinMagLinearMipPoint;
					else if (resourceTokens[1] == "aniso1x") { filterType = gpu_samplerfilter_type::Anisotropic; maxAnisotropy = 1; }
					else if (resourceTokens[1] == "aniso2x") { filterType = gpu_samplerfilter_type::Anisotropic; maxAnisotropy = 2; }
					else if (resourceTokens[1] == "aniso4x") { filterType = gpu_samplerfilter_type::Anisotropic; maxAnisotropy = 4; }
					else if (resourceTokens[1] == "aniso8x") { filterType = gpu_samplerfilter_type::Anisotropic; maxAnisotropy = 8; }
					else if (resourceTokens[1] == "aniso16x") { filterType = gpu_samplerfilter_type::Anisotropic; maxAnisotropy = 16; }
					else stl_assert(false);

					if (resourceTokens[2] == "clamp") clampType = gpu_texaddr_type::Clamp;
					else if (resourceTokens[2] == "border") clampType = gpu_texaddr_type::Border;
					else if (resourceTokens[2] == "mirror") clampType = gpu_texaddr_type::Mirror;
					else if (resourceTokens[2] == "wrap") clampType = gpu_texaddr_type::Wrap;
					else if (resourceTokens[2] == "mirroronce") clampType = gpu_texaddr_type::MirrorOnce;
					else stl_assert(false);

					auto sampler = gpu_rootsignature_desc::staticsampler::Create(resource.BindPoint, filterType, clampType, vis);
					sampler.MaxAnisotropy = maxAnisotropy;
					gfxRootSignature.StaticSamplers.push_back(sampler);
				}
			}
			else
			{
				samplerRanges.push_back(gpu_rootsignature_desc::parameter_range(gpu_rootsignature_desc::DrtSampler, resource.BindCount, resource.BindPoint, resource.Space));
				needsSamplerDescriptorTable = true;
			}
			break;
		case gpu_shaderreflection_desc::Texture:
		case gpu_shaderreflection_desc::TextureBuffer:
			// * Texture can't be directly in Root Signature!

			// * Parameter in descriptor table
			needsCbvSrvUavDescriptorTable = true;
			cbvSrvRanges.push_back(gpu_rootsignature_desc::parameter_range(gpu_rootsignature_desc::DrtSrv, resource.BindCount, resource.BindPoint, resource.Space));
			break;
		case gpu_shaderreflection_desc::Structured:
		case gpu_shaderreflection_desc::Byteaddress:
			
			if (resource.Name.size() > 2 && resource.Name.substr(0, 2) == "r_")
			{
				// * Parameter directly in Root Signature
				gfxRootSignature.Parameters.push_back(gpu_rootsignature_desc::parameter::SRV(resource.BindPoint, vis, resource.Space));
			}
			else
			{
				// * Parameter in descriptor table
				needsCbvSrvUavDescriptorTable = true;
				cbvSrvRanges.push_back(gpu_rootsignature_desc::parameter_range(gpu_rootsignature_desc::DrtSrv, resource.BindCount, resource.BindPoint, resource.Space));
			}
			break;
		case gpu_shaderreflection_desc::UavRwTyped:
		case gpu_shaderreflection_desc::UavRwStructured:
		case gpu_shaderreflection_desc::UavRwByteaddress:
		case gpu_shaderreflection_desc::UavAppendStructured:
		case gpu_shaderreflection_desc::UavConsumeStructured:
		case gpu_shaderreflection_desc::UavRwStructuredWithCounter:
			
			if (resource.Name.size() > 2 && resource.Name.substr(0, 2) == "r_")
			{
				// * Parameter directly in Root Signature
				gfxRootSignature.Parameters.push_back(gpu_rootsignature_desc::parameter::UAV(resource.BindPoint, vis, resource.Space));
			}
			else
			{
				// * Parameter in descriptor table
				needsCbvSrvUavDescriptorTable = true;
				cbvSrvRanges.push_back(gpu_rootsignature_desc::parameter_range(gpu_rootsignature_desc::DrtUav, resource.BindCount, resource.BindPoint, resource.Space));
			}

			break;
		}
	}

	if (needsCbvSrvUavDescriptorTable)
	{
		gfxRootSignature.Parameters.push_back(gpu_rootsignature_desc::parameter::DescriptorTable(&cbvSrvRanges[0], (int)cbvSrvRanges.size()));
	}

	if (needsSamplerDescriptorTable)
	{
		gfxRootSignature.Parameters.push_back(gpu_rootsignature_desc::parameter::DescriptorTable(&samplerRanges[0], (int)samplerRanges.size()));
	}

}

gfx_command_list* cfc::gfx_dx12::GetCommandList(usize commandListIdx)
{
	return m_impl->resCommandLists[commandListIdx].get();
}

gfx_command_list* cfc::gfx_dx12::GetCommandBundle(usize commandBundleIdx)
{
	return m_impl->resCommandBundles[commandBundleIdx].get();
}

usize cfc::gfx_dx12::GetBundleAllocator(usize bundleAllocatorIdx)
{
	return m_impl->resBundleCommandAllocator[bundleAllocatorIdx];
}

gfx_resource_stream* cfc::gfx_dx12::GetResourceStream(usize resourceStreamIdx)
{
	return m_impl->resResourceStreams[resourceStreamIdx].get();
}

gfx_descriptor_heap* cfc::gfx_dx12::GetDescriptorHeap(usize descriptorHeapIdx)
{
	return m_impl->resDescriptorHeaps[descriptorHeapIdx].get();
}

void gfx_dx12::_ExtractInputLayoutFromShader(const gpu_shaderreflection_desc& reflectedVS, gpu_graphicspipelinestate_desc& gfxPipeline)
{
	u32 byteOffset = 0;
	u32 inputElementID = 0;

	for (auto& input : reflectedVS.Inputs)
	{
		auto& outElement = gfxPipeline.InputElements[inputElementID++];
		u32 channels = input.ConvertMaskToNumChannels();
		u32 elementBytes = channels * 4; // all component types are 32 bits, so we can just multiply by 4.
		switch (input.ComponentType)
		{
		case gpu_shaderreflection_desc::component_type::Float32:
			if (channels == 1)		outElement.Format = gpu_format_type::R32Float;
			else if (channels == 2) outElement.Format = gpu_format_type::Rg32Float;
			else if (channels == 3) outElement.Format = gpu_format_type::Rgb32Float;
			else if (channels == 4) outElement.Format = gpu_format_type::Rgba32Float;
			break;
		case gpu_shaderreflection_desc::component_type::Sint32:
			if (channels == 1)		outElement.Format = gpu_format_type::R32Sint;
			else if (channels == 2) outElement.Format = gpu_format_type::Rg32Sint;
			else if (channels == 3) outElement.Format = gpu_format_type::Rgb32Sint;
			else if (channels == 4) outElement.Format = gpu_format_type::Rgba32Sint;
			break;
		case gpu_shaderreflection_desc::component_type::Uint32:
			if (channels == 1)		outElement.Format = gpu_format_type::R32Uint;
			else if (channels == 2) outElement.Format = gpu_format_type::Rg32Uint;
			else if (channels == 3) outElement.Format = gpu_format_type::Rgb32Uint;
			else if (channels == 4) outElement.Format = gpu_format_type::Rgba32Uint;
			break;
		case gpu_shaderreflection_desc::component_type::Unknown:
			stl_assert(input.ComponentType != gpu_shaderreflection_desc::component_type::Unknown);
			break;
		}
		outElement.SemanticIndex = input.SemanticIndex;
		outElement.SemanticName = input.SemanticName;
		outElement.AlignedByteOffset = byteOffset;
		byteOffset += elementBytes;
	}
}

u64 cfc::gfx_dx12::GetGlobalFrameIndex()
{
	return m_impl->frameCounter;
}

usize cfc::gfx_dx12::GetBackbufferFrameIndex()
{
	return m_impl->gpuCtx.SwapchainGetCurrentBackbufferIndex(m_impl->swapChain);
}

usize cfc::gfx_dx12::GetPreviousBackbufferFrameIndex()
{
	return (m_impl->gpuCtx.SwapchainGetCurrentBackbufferIndex(m_impl->swapChain) + (m_impl->numFrames - 1)) % m_impl->numFrames;
}

usize cfc::gfx_dx12::GetBackbufferFrameQuantity()
{
	return m_impl->numFrames;
}

usize cfc::gfx_dx12::GetTimerQueryResolvedFrameIndex()
{
	return (m_impl->gpuTimerQueryFrameCntr + (GetTimerQueryFrameDelayQuantity() - 2)) % GetTimerQueryFrameDelayQuantity();
}

usize cfc::gfx_dx12::GetTimerQueryWriteFrameIndex()
{
	return m_impl->gpuTimerQueryFrameCntr % GetTimerQueryFrameDelayQuantity();
}

usize cfc::gfx_dx12::GetTimerQueryFrameDelayQuantity()
{
	return TIMER_QUERIES_FRAMES_DELAY;
}

void cfc::gfx_dx12::QueueDestroy(gpu_object_type objectType, usize index)
{
	if (index == cfc::invalid_index)
		return;
	
	usize numFrames = GetBackbufferFrameQuantity();
	m_impl->mtxDeletionQueue.lock();
	m_impl->deletionQueue[GetGlobalFrameIndex() + numFrames].push_back(stl_pair<gpu_object_type, usize>(objectType, index));
	m_impl->mtxDeletionQueue.unlock();
}

gpu_dx12_context* gfx_dx12::DX12_GetContext()
{
	return &m_impl->gpuCtx;
}

gpu_dx12_cmdlist_direct_api* gfx_dx12::DX12_GetDirectCommandListAPI(usize cmdlistIdx)
{
	gfx_dx12_command_list* cmdlist = m_impl->resCommandLists[cmdlistIdx].get();
	return &cmdlist->cmdListDirectApi;
}

usize gfx_dx12::DX12_GetRootSignatureIdxFromProgram(usize gfxProgramIdx) const
{
	return this->m_impl->resGfxPrograms[gfxProgramIdx]->rootSignatureIndex;
}

void gfx_dx12::DX12_SetStablePowerState(bool stablePowerState)
{
	m_impl->gpuCtx.SetStablePowerState(stablePowerState);
}

}; // end namespace cfc