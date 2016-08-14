#pragma once

#include <cfc/base.h>
#include <cfc/stl/stl_vector.hpp>
#include <cfc/stl/stl_string.hpp>
#include <cfc/stl/stl_threading.hpp>
#include <cfc/gpu/gfx.h>

#include "renderPasses.h"
#include "occlusion.h"


namespace cfc
{
	// forward declare
	class gfx;
	class gfx_descriptor_heap;
	class gpu_dx12_cmdlist_indirect_api;
}; 

#define GRID_SIZE 4
#define DRAW_SPONZA 1

#if DRAW_SPONZA
#define MODEL_SCALE 0.001f
#else
#define MODEL_SCALE 1.0f
#endif

struct view_state;

struct occlusionDepthRT
{
	usize Width;
	usize Height;
	usize UAVRTResource;
};

struct material
{
	u32 AlbedoGFXResourceDescTableIndex = 0;
};

struct mat4_simple
{
	float Mat[16];
};

struct append_buffer
{
	usize AppendBufferGFXResourceIndex = cfc::invalid_index;
	usize AppendBufferCounterResetGfxResourceIndex = cfc::invalid_index;
	u32 CounterOffsetInBytes;
};

struct vertex_buffer
{
	usize GFXResourceIndex = cfc::invalid_index;
	u32 SizeInBytes;
	u32 StrideInBytes;
};

struct index_buffer
{
	usize GFXResourceIndex = cfc::invalid_index;
	u32 SizeInBytes;
	u32 NumIndices;
};

class scene
{
public:
	enum class OcclusionTypes
	{
		None,
		Gpu,
	};

	enum DebugRenderMode
	{
		NoDebugRender,
		RenderDebugPreviousDepth,
		RenderDebugReprojectedDepth,
		RenderDebugReprojectedDownSample,
		RenderDebugAll,
		Count
	};

public:
	void Load(cfc::context* const context, cfc::gfx& gfx, stl_string sceneFile);
	void Unload(cfc::gfx& gfx);

	void Resize(cfc::gfx& gfx);

	void Render(cfc::gfx& gfx, cfc::gfx_command_list& cmdList, OcclusionTypes occlusionType, usize viewStateGfxResourceIndex, usize prevViewStateGfxResourceIndex, view_state& view);

	void GatherFrameTimerQueries(cfc::gfx& gfx, OcclusionTypes occlusionType, stl_vector<cfc::gfx_gpu_timer_query>& timerQueriesOUT) const;

	DebugRenderMode GetDebugRenderMode() const { return m_debugRenderMode; }
	void SetDebugRenderMode(DebugRenderMode dbgRenderMode) { m_debugRenderMode = dbgRenderMode; }

	void AllowComputeAsVertexShader(bool allowed) { m_enableNvidiaVertexShaderTrick = allowed; }

	void AllowDownsampleAfterReproject(bool allowed) { m_enableReprojectedDownSample = allowed; }

	void AllowWireFrame(bool allowed) { m_rasterizeWireFrameOfVisibleGeometryAdditive = allowed; }

	stl_string GetStatus();

private:
	void renderNoOcclusion(cfc::gfx& gfx, cfc::gfx_command_list& cmdList, usize viewStateGfxResourceIndex, view_state& view);
	void renderGPUOcclusion(cfc::gfx& gfx, cfc::gfx_command_list& cmdList, usize viewStateGfxResourceIndex, usize prevViewStateGfxResourceIndex);

	void debugRenderTexture(cfc::gfx& gfx, cfc::gfx_command_list& cmdList, usize descriptorTableSrvIndex, u32 debugIndex);
	void setStatus(const stl_string& str);

private:
	cfc::gfx_descriptor_heap* m_opaqueRenderingDescHeap = nullptr;

	// DX12 interop (TODO: when we have a common execute indirect interface, remove this)
	cfc::gpu_dx12_cmdlist_indirect_api* m_opaqueIndirectCmdList = nullptr;

	// passes
	reproject_depth_buffer m_reprojectDepthBufferCmp;
	down_sample_reproject_depth_buffer m_downSampleReprojectedDepthBufferCmp;
	copy_reproject_depth_buffer m_copyReprojectDepthBuffer;
	visibility_draw_pass m_renderVisibilityGfx;
	sort_visibile_draw_calls m_collectVisibleDrawCallsCmp;
	sort_visible_draw_calls_VS m_collectVisibleDrawCallsCmpLowOverhead;
	opaque_draw_pass m_renderOpaqueGfx;
	full_screen_textured_quad m_debugFullScreenTexQuad;
	opaque_wireframe_pass m_debugOpaqueWireFrameGfx;

	// timer query groups indirect
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryIndirectDrawFrame;
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryClearUav;
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryReprojectDepth;
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryDownSampleReprojectedDepth;
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryCopyUAVToDepth;
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryDrawAABBs;
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryClearAppendBufferPass;
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryAquireVisibleObjects;
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryIndirectDraw;

	// timer query groups direct
	stl_vector<cfc::gfx_gpu_timer_query> m_timerQueryDirectDraw;

	// loading status
	stl_mutex m_mtxStatus;
	stl_string m_status;

	// visual fidelity resources
	u32 m_maxNumMeshesToRender = 0;
	stl_vector<vertex_buffer> m_vertexBuffers;
	stl_vector<index_buffer> m_indexBuffers;
	stl_vector<mat4_simple> m_modelMatrices;
	usize m_modelMatricesGFXResourceIndex = cfc::invalid_index;
	stl_vector<u32> m_materialIds;
	stl_vector<material> m_materials;
	stl_vector<usize> m_albedoTextureGFXResourceIndex;
	material m_defaultMaterial;

	stl_vector<usize> m_opaqueIndirectCmdListRef;
	stl_vector<append_buffer> m_opaqueIndirectCmdListAppend;
	stl_vector<usize> m_opaqueIndirectCmdListAppendDescTableOffset;

	// visibility culling resources
	cfc::gfx_descriptor_heap* m_depthBufferDescHeap = nullptr;
	occlusionDepthRT m_occlusionDepthBufferHalfRes;
	occlusionDepthRT m_occlusionDepthBufferQuarterRes;
	stl_vector<aabb> m_aabbs;
	stl_vector<aabb> m_final_aabbs;
	vertex_buffer m_aabbVertexBuffer;
	index_buffer m_aabbIndexBuffer;
	stl_vector<mat4_simple> m_aabbTransScaleMatrices; // can be optimized by only sending position and scale
	usize m_aabbTransScaleMatricesGFXResourceIndex = cfc::invalid_index;
	stl_vector<usize> m_visibilityBufferGFXResourceIndex;

	// debug
	usize m_debugRT = cfc::invalid_index;
	DebugRenderMode m_debugRenderMode = DebugRenderMode::NoDebugRender;
	bool m_enableNvidiaVertexShaderTrick = false;
	bool m_enableReprojectedDownSample = true;
	bool m_rasterizeWireFrameOfVisibleGeometryAdditive = false;
};