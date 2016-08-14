#include "gfx.h"

#include <cfc/core/io.h>

namespace cfc {

	void gfx_gpu_timer_query::Begin(gfx_command_list* const cmdList, const char* const description)
	{
		m_cmdList = cmdList;
		m_startIndex = cmdList->InsertTimerQuery();

		usize descriptionLength = strlen(description);
		stl_assert(descriptionLength < GFX_MAX_TIMER_QUERY_DESCRIPTION_STRING_SIZE);
		memcpy(m_description, description, descriptionLength);
	}

	void gfx_gpu_timer_query::Begin(gfx_command_list* const cmdList)
	{
		m_cmdList = cmdList;
		m_startIndex = cmdList->InsertTimerQuery();
	}

	void gfx_gpu_timer_query::End()
	{
		m_endIndex = m_cmdList->InsertTimerQuery();
	}


	usize gfx::AddShaderFromFile(context& ctx, const char* filename, const char* funcName /*= "main"*/, const char* shaderType /*= "vs_5_0"*/, const char* defineData /*=null*/)
	{
		iobuffer ioData = ctx.IO->ReadFileToMemory(filename);
		stl_assert(ioData); // read shader
		return AddShaderFromMemory(ioData.data, ioData.size, funcName, shaderType, filename, defineData);
	}

	gfx_barrier_list::gfx_barrier_list(usize reserve /* = 4*/)
	{
		Barriers.reserve(reserve);
	}

	void gfx_barrier_list::Reset()
	{
		Barriers.resize(0);
	}

	void gfx_barrier_list::BarrierResource(usize resourceIdx, u32 stateBefore, u32 stateAfter)
	{
		Barriers.push_back(gpu_resourcebarrier_desc::Transition(resourceIdx, stateBefore, stateAfter));
	}

	void gfx_barrier_list::BarrierUAV(usize resourceIdx)
	{
		Barriers.push_back(gpu_resourcebarrier_desc::UAV(resourceIdx));
	}

	void gfx_barrier_list::BarrierAliasing(usize resourceBeforeIdx, usize resourceAfterIdx)
	{
		Barriers.push_back(gpu_resourcebarrier_desc::Aliasing(resourceBeforeIdx, resourceAfterIdx));
	}

};