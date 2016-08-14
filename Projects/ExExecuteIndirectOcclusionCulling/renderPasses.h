#pragma once

#include <cfc/base.h>

// forward declare
namespace cfc
{
	class gfx_command_list;
	class gfx;
	struct context;
};

#define HALF_SCREEN_DIV 2
#define QUART_SCREEN_DIV 4


class render_pass
{
	virtual bool Load(cfc::context* const context, cfc::gfx& gfx) = 0;
	virtual void Unload(cfc::gfx& gfx) = 0;
	virtual void Begin(cfc::gfx_command_list* const cmndList) = 0;

	virtual usize GetShaderProgram() const = 0;
};

class compute_pass
{
	virtual bool Load(cfc::context* const context, cfc::gfx& gfx, u32 numThreadsX = 1, u32 numThreadsY = 1, u32 numThreadsZ = 1) = 0;
	virtual void Unload(cfc::gfx& gfx) = 0;
	virtual void Begin(cfc::gfx_command_list* const cmndList) = 0;

	virtual u32 GetNumThreadsX() const = 0;
	virtual u32 GetNumThreadsY() const = 0;
	virtual u32 GetNumThreadsZ() const = 0;

	virtual usize GetShaderProgram() const = 0;
};


class visibility_draw_pass : public render_pass
{
public:
	visibility_draw_pass();

	virtual bool Load(cfc::context* const context, cfc::gfx& gfx) override;
	virtual void Unload(cfc::gfx& gfx) override;

	virtual void Begin(cfc::gfx_command_list* const cmdList) override;

	virtual usize GetShaderProgram() const override { return m_shrDrawBBoxVisibilityProgram; }

private:
	usize m_shrDrawBBoxVisibilityVS = cfc::invalid_index;
	usize m_shrDrawBBoxVisibilityPS = cfc::invalid_index;
	usize m_shrDrawBBoxVisibilityProgram = cfc::invalid_index;
	usize m_shrDrawBBoxVisibilityProgramState = cfc::invalid_index;
};

class opaque_draw_pass : public render_pass
{
public:
	opaque_draw_pass();

	virtual bool Load(cfc::context* const context, cfc::gfx& gfx) override;
	virtual void Unload(cfc::gfx& gfx) override;

	virtual void Begin(cfc::gfx_command_list* const cmdList) override;

	virtual usize GetShaderProgram() const override { return m_shrDrawOpaqueProgram; }

private:
	usize m_shrDrawOpaqueVS = cfc::invalid_index;
	usize m_shrDrawOpaquePS = cfc::invalid_index;
	usize m_shrDrawOpaqueProgram = cfc::invalid_index;
	usize m_shrDrawOpaqueProgramState = cfc::invalid_index;
};

class opaque_wireframe_pass : public render_pass
{
public:
	opaque_wireframe_pass();

	virtual bool Load(cfc::context* const context, cfc::gfx& gfx) override;
	virtual void Unload(cfc::gfx& gfx) override;

	virtual void Begin(cfc::gfx_command_list* const cmdList) override;

	virtual usize GetShaderProgram() const override { return m_shrDrawOpaqueProgram; }

private:
	usize m_shrDrawOpaqueVS = cfc::invalid_index;
	usize m_shrDrawOpaquePS = cfc::invalid_index;
	usize m_shrDrawOpaqueProgram = cfc::invalid_index;
	usize m_shrDrawOpaqueProgramState = cfc::invalid_index;
};

class sort_visibile_draw_calls : public compute_pass
{
public:
	sort_visibile_draw_calls();

	virtual bool Load(cfc::context* const context, cfc::gfx& gfx, u32 numThreadsX = 1, u32 numThreadsY = 1, u32 numThreadsZ = 1) override;
	virtual void Unload(cfc::gfx& gfx) override;

	virtual void Begin(cfc::gfx_command_list* const cmdList) override;

	virtual usize GetShaderProgram() const override { return m_shrComputeProgram; }

	virtual u32 GetNumThreadsX() const override { return m_numThreadsX; };
	virtual u32 GetNumThreadsY() const override { return m_numThreadsY; };
	virtual u32 GetNumThreadsZ() const override { return m_numThreadsZ; };

private:
	usize m_shrComputeCS = cfc::invalid_index;
	usize m_shrComputeProgram = cfc::invalid_index;
	usize m_shrComputeProgramState = cfc::invalid_index;
	u32 m_numThreadsX = 0;
	u32 m_numThreadsY = 0;
	u32 m_numThreadsZ = 0;
};

class sort_visible_draw_calls_VS : public render_pass
{
public:
	sort_visible_draw_calls_VS();

	virtual bool Load(cfc::context* const context, cfc::gfx& gfx) override;
	virtual void Unload(cfc::gfx& gfx) override;

	virtual void Begin(cfc::gfx_command_list* const cmdList) override;

	virtual usize GetShaderProgram() const override { return m_shrCollectVisibleProgram; }

private:
	usize m_shrCollectVisibleVS = cfc::invalid_index;
	usize m_shrCollectVisiblePS = cfc::invalid_index;
	usize m_shrCollectVisibleProgram = cfc::invalid_index;
	usize m_shrCollectVisibleProgramState = cfc::invalid_index;
};

class reproject_depth_buffer : public compute_pass
{
public:
	reproject_depth_buffer();

	virtual bool Load(cfc::context* const context, cfc::gfx& gfx, u32 numThreadsX = 1, u32 numThreadsY = 1, u32 numThreadsZ = 1) override;
	virtual void Unload(cfc::gfx& gfx) override;

	virtual void Begin(cfc::gfx_command_list* const cmdList) override;

	virtual usize GetShaderProgram() const override { return m_shrComputeProgram; }

	virtual u32 GetNumThreadsX() const override { return m_numThreadsX; };
	virtual u32 GetNumThreadsY() const override { return m_numThreadsY; };
	virtual u32 GetNumThreadsZ() const override { return m_numThreadsZ; };

private:
	usize m_shrComputeCS = cfc::invalid_index;
	usize m_shrComputeProgram = cfc::invalid_index;
	usize m_shrComputeProgramState = cfc::invalid_index;
	u32 m_numThreadsX = 0;
	u32 m_numThreadsY = 0;
	u32 m_numThreadsZ = 0;
};

class down_sample_reproject_depth_buffer : public compute_pass
{
public:
	down_sample_reproject_depth_buffer();

	virtual bool Load(cfc::context* const context, cfc::gfx& gfx, u32 numThreadsX = 1, u32 numThreadsY = 1, u32 numThreadsZ = 1) override;
	virtual void Unload(cfc::gfx& gfx) override;

	virtual void Begin(cfc::gfx_command_list* const cmdList) override;

	virtual usize GetShaderProgram() const override { return m_shrComputeProgram; }

	virtual u32 GetNumThreadsX() const override { return m_numThreadsX; };
	virtual u32 GetNumThreadsY() const override { return m_numThreadsY; };
	virtual u32 GetNumThreadsZ() const override { return m_numThreadsZ; };

private:
	usize m_shrComputeCS = cfc::invalid_index;
	usize m_shrComputeProgram = cfc::invalid_index;
	usize m_shrComputeProgramState = cfc::invalid_index;
	u32 m_numThreadsX = 0;
	u32 m_numThreadsY = 0;
	u32 m_numThreadsZ = 0;
};

class copy_reproject_depth_buffer : public render_pass
{
public:
	copy_reproject_depth_buffer();

	virtual bool Load(cfc::context* const context, cfc::gfx& gfx) override;
	virtual void Unload(cfc::gfx& gfx) override;

	virtual void Begin(cfc::gfx_command_list* const cmdList) override;

	virtual usize GetShaderProgram() const override { return m_shrCopyDepthProgram; }

private:
	usize m_shrCopyDepthVS = cfc::invalid_index;
	usize m_shrCopyDepthPS = cfc::invalid_index;
	usize m_shrCopyDepthProgram = cfc::invalid_index;
	usize m_shrCopyDepthProgramState = cfc::invalid_index;
};

class full_screen_textured_quad : public render_pass
{
public:
	full_screen_textured_quad();

	virtual bool Load(cfc::context* const context, cfc::gfx& gfx) override;
	virtual void Unload(cfc::gfx& gfx) override;

	virtual void Begin(cfc::gfx_command_list* const cmdList) override;

	virtual usize GetShaderProgram() const override { return m_shrProgram; }

private:
	usize m_shrVS = cfc::invalid_index;
	usize m_shrPS = cfc::invalid_index;
	usize m_shrProgram = cfc::invalid_index;
	usize m_shrProgramState = cfc::invalid_index;
};