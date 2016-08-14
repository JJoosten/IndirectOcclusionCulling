#pragma once

#include <cfc/base.h>
#include <cfc/core/context.h>
#include <cfc/gpu/gpu.h>
#include <cfc/gpu/gfx.h>

#include <cfc/stl/stl_pimpl.hpp>

namespace cfc {

	struct _imp_dx12_gfx;
	class gpu_dx12_context;
	class gpu_dx12_cmdlist_direct_api;

	class CFC_API gfx_dx12 : public gfx
	{
	public:
		gfx_dx12();
		virtual ~gfx_dx12();

		virtual void Init(context& ctx, usize deviceID = 0, u32 numFrames = 2, gpu_swapimage_type imgType = gpu_swapimage_type::Rgba8Unorm, gpu_format_type depthType = gpu_format_type::D32Float, gpu_swapflip_type flipType = gpu_swapflip_type::Discard);
		virtual gpu_features GetDeviceFeatures() const override;

		virtual usize AddCommandList() override;
		virtual usize AddCommandBundle(usize cmdAllocatorBundleId) override;
		virtual usize AddBundleAllocator() override;
		virtual usize AddResourceStream() override;
		virtual usize AddDescriptorHeap(i32 maxCbvSrvUav, i32 maxSamplers) override;
		virtual usize AddShaderFromMemory(const void* fileData, usize fileDataSize, const char* funcName = "main", const char* shaderType = "vs_5_0", const char* shaderFilename = "unknown.shd", const char* defineData = nullptr) override;
		virtual usize AddGraphicsProgram(usize vertexShader = cfc::invalid_index, usize pixelShader = cfc::invalid_index, usize geometryShader = cfc::invalid_index, usize hullShader = cfc::invalid_index, usize domainShader = cfc::invalid_index) override;
		virtual usize AddGraphicsProgramPipelineState(usize graphicsProgramIdx, const gfx_gfxprogram_desc& descriptor) override;
		virtual usize AddComputeProgram(usize computeShader = cfc::invalid_index) override;
		virtual usize AddComputeProgramPipelineState(usize computeProgramIdx, const gfx_cmpprogram_desc& descriptor) override;
		virtual usize AddRenderTarget2D(i32 width, i32 height, gpu_format_type format, gpu_defaultclear_desc defaultClear = gpu_defaultclear_desc()) override;

		virtual void RemoveShader(usize shaderIdx) override;
		virtual void RemoveResource(usize bufferIdx) override;
		virtual void RemoveGraphicsProgram(usize gfxProgramIdx) override;
		virtual void RemoveComputeProgram(usize cmpProgramIdx) override;
		virtual void RemoveDescriptorHeap(usize descriptorHeapIdx) override;
		virtual void RemoveCommandList(usize commandListIdx) override;
		virtual void RemoveCommandBundle(usize commandBundleIdx) override;
		virtual void RemoveBundleAllocator(usize bundleAllocatorIdx) override;
		virtual void RemoveResourceStream(usize resourceStream) override;
		virtual void RemoveRenderTarget(usize rtID) override;

		virtual void WaitForGpu();

		virtual void ExecuteCommandLists(const usize* commandLists, usize numCommandLists) override;
		virtual bool Present(u32 swapInterval = 1, u32 flags = 0) override;

		virtual void ResolveTimerQueries() override;
		virtual void ReadBackTimerQueryResults() override;
		virtual f64  GetTimerQueryResultInMS(const u32 index) override;
		virtual f64  GetTimerQueryResultInMS(const gfx_gpu_timer_query& timerQuery) override;

		virtual gfx_descriptor_heap* GetDescriptorHeap(usize descriptorHeapIdx) override;
		virtual gfx_resource_stream* GetResourceStream(usize resourceStreamIdx) override;
		virtual gfx_command_list* GetCommandList(usize commandListIdx) override;
		virtual gfx_command_list* GetCommandBundle(usize commandBufferIdx) override;
		virtual usize GetBundleAllocator(usize bundleAllocatorIdx) override;

		virtual u32 GetGraphicsProgramBindLocation(usize gfxProgramIdx, const char* name) override;
		virtual u32 GetComputeProgramBindLocation(usize cmpProgramIdx, const char* name) override;

		virtual usize GetRenderTargetResource(usize rtID) override;
		virtual usize GetRenderTargetRTVOffset(usize rtID) override;
		virtual usize GetRenderTargetDSVOffset(usize rtID) override;
		virtual i32 GetRenderTargetWidth(usize rtID) override;
		virtual i32 GetRenderTargetHeight(usize rtID) override;
		virtual gpu_format_type GetRenderTargetFormat(usize rtID) override;

		virtual i32 GetBackbufferWidth() override;
		virtual i32 GetBackbufferHeight() override;
		virtual usize GetBackbufferRTResource() override;
		virtual usize GetBackbufferRTResource(usize index) override;
		virtual usize GetBackbufferDSResource() override;
		virtual usize GetBackbufferRTVOffset() override;
		virtual usize GetBackbufferDSVOffset() override;
		virtual gpu_format_type GetBackbufferRTVFormat() override;
		virtual gpu_format_type GetBackbufferDSVFormat() override;
		virtual usize GetBackbufferFrameIndex() override;
		virtual usize GetPreviousBackbufferFrameIndex() override;
		virtual usize GetBackbufferFrameQuantity() override;
		virtual u64 GetGlobalFrameIndex() override;

		virtual usize GetTimerQueryResolvedFrameIndex() override;
		virtual usize GetTimerQueryWriteFrameIndex() override;
		virtual usize GetTimerQueryFrameDelayQuantity() override;

		void QueueDestroy(gpu_object_type objectType, usize index);

		// extensions for gfx layer to DX12 interop
		gpu_dx12_context* DX12_GetContext();
		gpu_dx12_cmdlist_direct_api* DX12_GetDirectCommandListAPI(usize cmdlistIdx);
		usize DX12_GetRootSignatureIdxFromProgram(usize gfxProgramIdx) const;
		void DX12_SetStablePowerState(bool stablePowerState);

	protected:
		void _Resize();
		void _ExtractInputLayoutFromShader(const gpu_shaderreflection_desc& reflectedVS, gpu_graphicspipelinestate_desc& gfxPipeline);
		void _ExtractRootSignatureFromShaders(const gpu_shaderreflection_desc** reflectedShaders, gpu_shadervisibility_type* visibilityTypes, int numShaders, gpu_rootsignature_desc& gfxRootSignature);
		usize _AddProgram(usize vertexShader, usize pixelShader, usize geometryShader, usize hullShader, usize domainShader, usize computeShader);

		stl_pimpl<_imp_dx12_gfx, 8196> m_impl;
	};

}; // end namespace cfc