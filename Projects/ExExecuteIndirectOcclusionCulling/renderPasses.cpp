#include "renderPasses.h"

#include <cfc/gpu/gfx.h>

visibility_draw_pass::visibility_draw_pass()
{

}

bool visibility_draw_pass::Load(cfc::context* const context, cfc::gfx& gfx)
{
	stl_assert(context);

	// simple temp define buffer
	char shaderDefines[256];

	// load rasterize occlusion cull shader
	sprintf(shaderDefines, " ");
	m_shrDrawBBoxVisibilityVS = gfx.AddShaderFromFile(*context, "drawVisibilityPass.hlsl", "VSMain", "vs_5_0", shaderDefines);
	m_shrDrawBBoxVisibilityPS = gfx.AddShaderFromFile(*context, "drawVisibilityPass.hlsl", "PSMain", "ps_5_0", shaderDefines);
	m_shrDrawBBoxVisibilityProgram = gfx.AddGraphicsProgram(m_shrDrawBBoxVisibilityVS, m_shrDrawBBoxVisibilityPS);

	cfc::gfx_gfxprogram_desc dsc;
	auto deviceFeatures = gfx.GetDeviceFeatures();
	if (deviceFeatures.ConservativeRasterizationTier != cfc::gpu_features::conservativerst_type::None)
		dsc.Pipeline.RasterizerState.ConservativeRaster = cfc::gpu_graphicspipelinestate_desc::rasterizer_desc::CrmOn;
	dsc.Pipeline.DSVFormat = gfx.GetBackbufferDSVFormat();
	dsc.Pipeline.DepthStencilState.DepthEnable = true;
	dsc.Pipeline.DepthStencilState.DepthWriteMask = cfc::gpu_depthwritemask_type::Zero;
	dsc.Pipeline.DepthStencilState.DepthFunc = cfc::gpu_comparisonfunc_type::LessEqual;
	dsc.Pipeline.PrimitiveTopology = cfc::gpu_primitivetopology_type::Triangle;
	dsc.Pipeline.RasterizerState.CullMode = cfc::gpu_graphicspipelinestate_desc::rasterizer_desc::CullNone;
	m_shrDrawBBoxVisibilityProgramState = gfx.AddGraphicsProgramPipelineState(m_shrDrawBBoxVisibilityProgram, dsc);
	
	return true;
}

void visibility_draw_pass::Unload(cfc::gfx& gfx)
{
	gfx.RemoveGraphicsProgram(m_shrDrawBBoxVisibilityProgram);
	gfx.RemoveShader(m_shrDrawBBoxVisibilityPS);
	gfx.RemoveShader(m_shrDrawBBoxVisibilityVS);
}

void visibility_draw_pass::Begin(cfc::gfx_command_list* const cmdList)
{
	stl_assert(cmdList);

	cmdList->GFXSetProgram(m_shrDrawBBoxVisibilityProgram);
	cmdList->GFXSetProgramState(m_shrDrawBBoxVisibilityProgram, m_shrDrawBBoxVisibilityProgramState);
}


opaque_draw_pass::opaque_draw_pass()
{

}

bool opaque_draw_pass::Load(cfc::context* const context, cfc::gfx& gfx)
{
	stl_assert(context);

	// simple temp define buffer
	char shaderDefines[256];
	sprintf(shaderDefines, " ");

	// NOTE: that the PS uses shader model 5.1 since we use descriptor table arrays for the textures
	m_shrDrawOpaqueVS = gfx.AddShaderFromFile(*context, "drawOpaquePass.hlsl", "VSMain", "vs_5_0", shaderDefines);
	m_shrDrawOpaquePS = gfx.AddShaderFromFile(*context, "drawOpaquePass.hlsl", "PSMain", "ps_5_1", shaderDefines);
	m_shrDrawOpaqueProgram = gfx.AddGraphicsProgram(m_shrDrawOpaqueVS, m_shrDrawOpaquePS);

	cfc::gfx_gfxprogram_desc dsc;
	dsc.Pipeline.DSVFormat = gfx.GetBackbufferDSVFormat();
	dsc.Pipeline.NumRenderTargets = 1;
	dsc.Pipeline.RTVFormats[0] = gfx.GetBackbufferRTVFormat();
	dsc.Pipeline.DepthStencilState.DepthEnable = true;
	dsc.Pipeline.PrimitiveTopology = cfc::gpu_primitivetopology_type::Triangle;
	dsc.Pipeline.RasterizerState.CullMode = cfc::gpu_graphicspipelinestate_desc::rasterizer_desc::CullBack;
	m_shrDrawOpaqueProgramState = gfx.AddGraphicsProgramPipelineState(m_shrDrawOpaqueProgram, dsc);

	return true;
}

void opaque_draw_pass::Unload(cfc::gfx& gfx)
{
	gfx.RemoveGraphicsProgram(m_shrDrawOpaqueProgram);
	gfx.RemoveShader(m_shrDrawOpaquePS);
	gfx.RemoveShader(m_shrDrawOpaqueVS);
}

void opaque_draw_pass::Begin(cfc::gfx_command_list* const cmdList)
{
	stl_assert(cmdList);
	
	cmdList->GFXSetProgram(m_shrDrawOpaqueProgram);
	cmdList->GFXSetProgramState(m_shrDrawOpaqueProgram, m_shrDrawOpaqueProgramState);
}


opaque_wireframe_pass::opaque_wireframe_pass()
{

}

bool opaque_wireframe_pass::Load(cfc::context* const context, cfc::gfx& gfx)
{
	stl_assert(context);

	// simple temp define buffer
	char shaderDefines[256];
	sprintf(shaderDefines, " ");

	// NOTE: that the PS uses shader model 5.1 since we use descriptor table arrays for the textures
	m_shrDrawOpaqueVS = gfx.AddShaderFromFile(*context, "drawOpaquePass.hlsl", "VSMain", "vs_5_0", shaderDefines);
	m_shrDrawOpaquePS = gfx.AddShaderFromFile(*context, "drawOpaquePass.hlsl", "PSWireFrame", "ps_5_1", shaderDefines);
	m_shrDrawOpaqueProgram = gfx.AddGraphicsProgram(m_shrDrawOpaqueVS, m_shrDrawOpaquePS);

	cfc::gfx_gfxprogram_desc dsc;
	dsc.Pipeline.NumRenderTargets = 1;
	dsc.Pipeline.RTVFormats[0] = gfx.GetBackbufferRTVFormat();
	dsc.Pipeline.DepthStencilState.DepthEnable = false;
	dsc.Pipeline.DepthStencilState.DepthWriteMask = cfc::gpu_depthwritemask_type::Zero;
	dsc.Pipeline.PrimitiveTopology = cfc::gpu_primitivetopology_type::Triangle;
	dsc.Pipeline.RasterizerState.CullMode = cfc::gpu_graphicspipelinestate_desc::rasterizer_desc::CullBack;
	dsc.Pipeline.RasterizerState.FillMode = cfc::gpu_graphicspipelinestate_desc::rasterizer_desc::FillWireframe;
	m_shrDrawOpaqueProgramState = gfx.AddGraphicsProgramPipelineState(m_shrDrawOpaqueProgram, dsc);

	return true;
}

void opaque_wireframe_pass::Unload(cfc::gfx& gfx)
{
	gfx.RemoveGraphicsProgram(m_shrDrawOpaqueProgram);
	gfx.RemoveShader(m_shrDrawOpaquePS);
	gfx.RemoveShader(m_shrDrawOpaqueVS);
}

void opaque_wireframe_pass::Begin(cfc::gfx_command_list* const cmdList)
{
	stl_assert(cmdList);

	cmdList->GFXSetProgram(m_shrDrawOpaqueProgram);
	cmdList->GFXSetProgramState(m_shrDrawOpaqueProgram, m_shrDrawOpaqueProgramState);
}


sort_visibile_draw_calls::sort_visibile_draw_calls()
{

}

bool sort_visibile_draw_calls::Load(cfc::context* const context, cfc::gfx& gfx, u32 numThreadsX /*= 1*/, u32 numThreadsY /*= 1*/, u32 numThreadsZ /*= 1*/)
{
	stl_assert(context);

	m_numThreadsX = numThreadsX;
	m_numThreadsY = numThreadsY;
	m_numThreadsZ = numThreadsZ;

	// simple temp define buffer
	char shaderDefines[256];

	int offset = sprintf(shaderDefines, "#define NUM_THREADS_X %d \n", numThreadsX);
	offset += sprintf(shaderDefines + offset, "#define NUM_THREADS_Y %d \n", numThreadsY);
	offset += sprintf(shaderDefines + offset, "#define NUM_THREADS_Z %d \n", numThreadsZ);
	m_shrComputeCS = gfx.AddShaderFromFile(*context, "sortVisibleObjects.hlsl", "CSMain", "cs_5_0", shaderDefines);
	m_shrComputeProgram = gfx.AddComputeProgram(m_shrComputeCS);

	cfc::gfx_cmpprogram_desc dsc;
	m_shrComputeProgramState = gfx.AddComputeProgramPipelineState(m_shrComputeProgram, dsc);

	return true;
}

void sort_visibile_draw_calls::Unload(cfc::gfx& gfx)
{
	gfx.RemoveComputeProgram(m_shrComputeProgram);
	gfx.RemoveShader(m_shrComputeCS);
}

void sort_visibile_draw_calls::Begin(cfc::gfx_command_list* const cmdList)
{
	stl_assert(cmdList);

	cmdList->CMPSetProgram(m_shrComputeProgram);
	cmdList->CMPSetProgramState(m_shrComputeProgram, m_shrComputeProgramState);

}


sort_visible_draw_calls_VS::sort_visible_draw_calls_VS()
{

}

bool sort_visible_draw_calls_VS::Load(cfc::context* const context, cfc::gfx& gfx)
{
	stl_assert(context);

	// simple temp define buffer
	char shaderDefines[256];

	// load rasterize occlusion cull shader
	int offset = sprintf(shaderDefines, "#define NUM_THREADS_X %d \n", 1);
	offset += sprintf(shaderDefines + offset, "#define NUM_THREADS_Y %d \n", 1);
	offset += sprintf(shaderDefines + offset, "#define NUM_THREADS_Z %d \n", 1);
	m_shrCollectVisibleVS = gfx.AddShaderFromFile(*context, "sortVisibleObjects.hlsl", "VSMain", "vs_5_0", shaderDefines);
	m_shrCollectVisibleProgram = gfx.AddGraphicsProgram(m_shrCollectVisibleVS);

	cfc::gfx_gfxprogram_desc dsc;
	dsc.Pipeline.DepthStencilState.DepthEnable = false;
	dsc.Pipeline.PrimitiveTopology = cfc::gpu_primitivetopology_type::Point;
	m_shrCollectVisibleProgramState = gfx.AddGraphicsProgramPipelineState(m_shrCollectVisibleProgram, dsc);

	return true;
}

void sort_visible_draw_calls_VS::Unload(cfc::gfx& gfx)
{
	gfx.RemoveGraphicsProgram(m_shrCollectVisibleProgram);
	gfx.RemoveShader(m_shrCollectVisibleVS);
}

void sort_visible_draw_calls_VS::Begin(cfc::gfx_command_list* const cmdList)
{
	stl_assert(cmdList);

	cmdList->GFXSetProgram(m_shrCollectVisibleProgram);
	cmdList->GFXSetProgramState(m_shrCollectVisibleProgram, m_shrCollectVisibleProgramState);
}


reproject_depth_buffer::reproject_depth_buffer()
{

}

bool reproject_depth_buffer::Load(cfc::context* const context, cfc::gfx& gfx, u32 numThreadsX /*= 1*/, u32 numThreadsY /*= 1*/, u32 numThreadsZ /*= 1*/)
{
	stl_assert(context);

	m_numThreadsX = numThreadsX;
	m_numThreadsY = numThreadsY;
	m_numThreadsZ = numThreadsZ;

	// simple temp define buffer
	char shaderDefines[256];

	int offset = sprintf(shaderDefines, "#define NUM_THREADS_X %d \n", numThreadsX);
	offset += sprintf(shaderDefines + offset, "#define NUM_THREADS_Y %d \n", numThreadsY);
	offset += sprintf(shaderDefines + offset, "#define NUM_THREADS_Z %d \n", numThreadsZ);
	offset += sprintf(shaderDefines + offset, "#define HALF_SCREEN_WIDTH %d \n", gfx.GetBackbufferWidth() / HALF_SCREEN_DIV);
	offset += sprintf(shaderDefines + offset, "#define HALF_SCREEN_HEIGHT %d \n", gfx.GetBackbufferHeight() / HALF_SCREEN_DIV);
	m_shrComputeCS = gfx.AddShaderFromFile(*context, "reprojectDepth.hlsl", "CSMain", "cs_5_0", shaderDefines);
	
	if (m_shrComputeCS == cfc::invalid_index)
		return false;
	
	m_shrComputeProgram = gfx.AddComputeProgram(m_shrComputeCS);

	if (m_shrComputeProgram == cfc::invalid_index)
		return false;

	cfc::gfx_cmpprogram_desc dsc;
	m_shrComputeProgramState = gfx.AddComputeProgramPipelineState(m_shrComputeProgram, dsc);

	if (m_shrComputeProgramState == cfc::invalid_index)
		return false;


	return true;
}

void reproject_depth_buffer::Unload(cfc::gfx& gfx)
{
	gfx.RemoveComputeProgram(m_shrComputeProgram);
	gfx.RemoveShader(m_shrComputeCS);
}

void reproject_depth_buffer::Begin(cfc::gfx_command_list* const cmdList)
{
	stl_assert(cmdList);

	cmdList->CMPSetProgram(m_shrComputeProgram);
	cmdList->CMPSetProgramState(m_shrComputeProgram, m_shrComputeProgramState);
}


down_sample_reproject_depth_buffer::down_sample_reproject_depth_buffer()
{

}

bool down_sample_reproject_depth_buffer::Load(cfc::context* const context, cfc::gfx& gfx, u32 numThreadsX /*= 1*/, u32 numThreadsY /*= 1*/, u32 numThreadsZ /*= 1*/)
{
	stl_assert(context);

	m_numThreadsX = numThreadsX;
	m_numThreadsY = numThreadsY;
	m_numThreadsZ = numThreadsZ;

	// simple temp define buffer
	char shaderDefines[256];

	int offset = sprintf(shaderDefines, "#define NUM_THREADS_X %d \n", numThreadsX);
	offset += sprintf(shaderDefines + offset, "#define NUM_THREADS_Y %d \n", numThreadsY);
	offset += sprintf(shaderDefines + offset, "#define NUM_THREADS_Z %d \n", numThreadsZ);
	m_shrComputeCS = gfx.AddShaderFromFile(*context, "reprojectDepthDownSample.hlsl", "CSMain", "cs_5_0", shaderDefines);

	if (m_shrComputeCS == cfc::invalid_index)
		return false;

	m_shrComputeProgram = gfx.AddComputeProgram(m_shrComputeCS);

	if (m_shrComputeProgram == cfc::invalid_index)
		return false;

	cfc::gfx_cmpprogram_desc dsc;
	m_shrComputeProgramState = gfx.AddComputeProgramPipelineState(m_shrComputeProgram, dsc);

	if (m_shrComputeProgramState == cfc::invalid_index)
		return false;


	return true;
}

void down_sample_reproject_depth_buffer::Unload(cfc::gfx& gfx)
{
	gfx.RemoveComputeProgram(m_shrComputeProgram);
	gfx.RemoveShader(m_shrComputeCS);
}

void down_sample_reproject_depth_buffer::Begin(cfc::gfx_command_list* const cmdList)
{
	stl_assert(cmdList);

	cmdList->CMPSetProgram(m_shrComputeProgram);
	cmdList->CMPSetProgramState(m_shrComputeProgram, m_shrComputeProgramState);
}


copy_reproject_depth_buffer::copy_reproject_depth_buffer()
{

}

bool copy_reproject_depth_buffer::Load(cfc::context* const context, cfc::gfx& gfx)
{
	stl_assert(context);

	// load rasterize occlusion cull shader
	m_shrCopyDepthVS = gfx.AddShaderFromFile(*context, "reprojectDepthCopy.hlsl", "VSMain", "vs_5_0", nullptr);
	m_shrCopyDepthPS = gfx.AddShaderFromFile(*context, "reprojectDepthCopy.hlsl", "PSMain", "ps_5_0", nullptr);
	m_shrCopyDepthProgram = gfx.AddGraphicsProgram(m_shrCopyDepthVS, m_shrCopyDepthPS);

	cfc::gfx_gfxprogram_desc dsc;
	dsc.Pipeline.DSVFormat = gfx.GetBackbufferDSVFormat();
	dsc.Pipeline.DepthStencilState.DepthEnable = true;
	dsc.Pipeline.DepthStencilState.DepthWriteMask = cfc::gpu_depthwritemask_type::All;
	dsc.Pipeline.DepthStencilState.DepthFunc = cfc::gpu_comparisonfunc_type::Always;
	dsc.Pipeline.PrimitiveTopology = cfc::gpu_primitivetopology_type::Triangle;
	m_shrCopyDepthProgramState = gfx.AddGraphicsProgramPipelineState(m_shrCopyDepthProgram, dsc);

	return true;
}

void copy_reproject_depth_buffer::Unload(cfc::gfx& gfx)
{
	gfx.RemoveGraphicsProgram(m_shrCopyDepthProgram);
	gfx.RemoveShader(m_shrCopyDepthVS);
	gfx.RemoveShader(m_shrCopyDepthPS);
}

void copy_reproject_depth_buffer::Begin(cfc::gfx_command_list* const cmdList)
{
	stl_assert(cmdList);

	cmdList->GFXSetProgram(m_shrCopyDepthProgram);
	cmdList->GFXSetProgramState(m_shrCopyDepthProgram, m_shrCopyDepthProgramState);
}


full_screen_textured_quad::full_screen_textured_quad()
{

}

bool full_screen_textured_quad::Load(cfc::context* const context, cfc::gfx& gfx)
{
	stl_assert(context);

	// load rasterize occlusion cull shader
	m_shrVS = gfx.AddShaderFromFile(*context, "fullScreenTexture.hlsl", "VSMain", "vs_5_0", nullptr);
	m_shrPS = gfx.AddShaderFromFile(*context, "fullScreenTexture.hlsl", "PSMain", "ps_5_0", nullptr);
	m_shrProgram = gfx.AddGraphicsProgram(m_shrVS, m_shrPS);

	cfc::gfx_gfxprogram_desc dsc;
	dsc.Pipeline.RTVFormats[0] = gfx.GetBackbufferRTVFormat();
	dsc.Pipeline.NumRenderTargets = 1;
	dsc.Pipeline.DepthStencilState.DepthEnable = false;
	dsc.Pipeline.PrimitiveTopology = cfc::gpu_primitivetopology_type::Triangle;
	dsc.Pipeline.BlendState.RenderTarget->BlendEnabled = true;
	dsc.Pipeline.BlendState.RenderTarget[0].SrcBlend = cfc::gpu_blendmode_type::One;
	dsc.Pipeline.BlendState.RenderTarget[0].SrcBlendAlpha = cfc::gpu_blendmode_type::One;
	dsc.Pipeline.BlendState.RenderTarget[0].DstBlend = cfc::gpu_blendmode_type::InvSrcAlpha;
	dsc.Pipeline.BlendState.RenderTarget[0].DstBlendAlpha = cfc::gpu_blendmode_type::InvSrcAlpha;
	m_shrProgramState = gfx.AddGraphicsProgramPipelineState(m_shrProgram, dsc);

	return true;
}

void full_screen_textured_quad::Unload(cfc::gfx& gfx)
{
	gfx.RemoveGraphicsProgram(m_shrProgram);
	gfx.RemoveShader(m_shrVS);
	gfx.RemoveShader(m_shrPS);
}

void full_screen_textured_quad::Begin(cfc::gfx_command_list* const cmdList)
{
	stl_assert(cmdList);

	cmdList->GFXSetProgram(m_shrProgram);
	cmdList->GFXSetProgramState(m_shrProgram, m_shrProgramState);
}
