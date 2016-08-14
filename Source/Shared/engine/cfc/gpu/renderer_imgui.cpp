// ImGui Win32 + DirectX11 binding
// In this binding, ImTextureID is used to store a 'ID3D11ShaderResourceView*' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include <dependencies/imgui/imgui.h>
#include "renderer_imgui.h"

#include "../core/context.h"
#include "gfx.h"

#include "../platform/platform_win32.hpp"

#define CB_ALIGNMENT_IN_BYTES 256


static cfc::context*	g_context = nullptr;

static cfc::gfx*	g_gfx = nullptr;
static cfc::gfx_descriptor_heap* g_descriptorHeap = nullptr;
static cfc::gfx_resource_stream* g_resourceStream = nullptr;
static stl_vector<cfc::gfx_command_list*> g_commandLists;

static bool				g_isInitialized = false;
static usize			g_vb = cfc::invalid_index;
static usize			g_ib = cfc::invalid_index;
static usize			g_shaderProgram = cfc::invalid_index;
static usize			g_vs = cfc::invalid_index;
static usize			g_ps = cfc::invalid_index;
static usize			g_pso = cfc::invalid_index;
static usize			g_constantBuffer = cfc::invalid_index;
static usize			g_fontTexture = cfc::invalid_index;
static void*			g_vbCpuStagingMemory = nullptr;
static void*			g_ibCpuStagingMemory = nullptr;
static i32              g_VertexBufferSize = 5000;
static i32				g_IndexBufferSize = 10000;
static u64              g_Time = 0;
static u64              g_TicksPerSecond = 0;

cfc::window::event evKeyDown;
cfc::window::event evKeyUp;
cfc::window::event evCursorDown;
cfc::window::event evCursorUp;
cfc::window::event evCursorScroll;
cfc::window::event evCursorMove;
cfc::window::event evAddCharacter;


struct VERTEX_CONSTANT_BUFFER
{
    float        mvp[4][4];
};

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplGfx_RenderDrawLists(ImDrawData* draw_data)
{
	const usize currentFrame = g_gfx->GetBackbufferFrameIndex();
	cfc::gfx_command_list& currentCmdList = *g_commandLists[currentFrame];
	currentCmdList.Reset();

	currentCmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(g_gfx->GetBackbufferRTResource(), cfc::gpu_resourcestate::Present, cfc::gpu_resourcestate::RenderTarget));
	
	currentCmdList.GFXSetRenderTargets(g_gfx->GetBackbufferRTVOffset(), cfc::invalid_index);

    // Create and grow vertex/index buffers if needed
    if (g_vb == cfc::invalid_index || g_VertexBufferSize < draw_data->TotalVtxCount)
    {
		if (g_vb != cfc::invalid_index)
		{
			g_gfx->RemoveResource(g_vb);
			g_vb = cfc::invalid_index;
			delete g_vbCpuStagingMemory;
		}

        g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
		g_vb = g_resourceStream->AddDynamicResource(cfc::gfx_resource_type::VertexBuffer, g_VertexBufferSize * sizeof(ImDrawVert) * g_gfx->GetBackbufferFrameQuantity(), true);
		g_resourceStream->Flush();
		g_vbCpuStagingMemory = new char[g_VertexBufferSize * sizeof(ImDrawVert) * g_gfx->GetBackbufferFrameQuantity()];
	}
	if (g_ib == cfc::invalid_index || g_IndexBufferSize < draw_data->TotalIdxCount)
    {
		if (g_ib != cfc::invalid_index)
		{
			g_gfx->RemoveResource(g_ib);
			g_ib = cfc::invalid_index;
			delete g_ibCpuStagingMemory;
		}

        g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
		g_ib = g_resourceStream->AddDynamicResource(cfc::gfx_resource_type::IndexBuffer, g_IndexBufferSize * sizeof(ImDrawIdx) * g_gfx->GetBackbufferFrameQuantity(), true);
		g_resourceStream->Flush();
		g_ibCpuStagingMemory = new char[g_IndexBufferSize * sizeof(ImDrawIdx) * g_gfx->GetBackbufferFrameQuantity()];
    }

	// Copy and convert all vertices into a single contiguous buffer
	{
		usize vbUpdateSizeInBytes = 0;
		usize ibUpdateSizeInBytes = 0;

		ImDrawVert* vtx_dst = (ImDrawVert*)g_vbCpuStagingMemory;
		ImDrawIdx* idx_dst = (ImDrawIdx*)g_ibCpuStagingMemory;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			memcpy(vtx_dst, &cmd_list->VtxBuffer[0], cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
			memcpy(idx_dst, &cmd_list->IdxBuffer[0], cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
			vtx_dst += cmd_list->VtxBuffer.size();
			idx_dst += cmd_list->IdxBuffer.size();
			vbUpdateSizeInBytes += cmd_list->VtxBuffer.size() * sizeof(ImDrawVert);
			ibUpdateSizeInBytes += cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx);
		}

		g_resourceStream->UpdateDynamicResource(g_vb, vbUpdateSizeInBytes, g_vbCpuStagingMemory, g_VertexBufferSize * sizeof(ImDrawVert) * currentFrame);
		g_resourceStream->UpdateDynamicResource(g_ib, ibUpdateSizeInBytes, g_ibCpuStagingMemory, g_IndexBufferSize * sizeof(ImDrawIdx) * currentFrame);
	}

    // Setup orthographic projection matrix into our constant buffer
    {
        float L = 0.0f;
        float R = ImGui::GetIO().DisplaySize.x;
        float B = ImGui::GetIO().DisplaySize.y;
        float T = 0.0f;
        float mvp[4][4] =
        {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };

		g_resourceStream->UpdateDynamicResource(g_constantBuffer, sizeof(VERTEX_CONSTANT_BUFFER), &mvp, CB_ALIGNMENT_IN_BYTES * currentFrame);
	}

	g_resourceStream->Flush();

    // Setup gpu_viewport
	currentCmdList.GFXSetViewports(cfc::gpu_viewport(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f));

	currentCmdList.GFXSetProgram(g_shaderProgram);
	currentCmdList.GFXSetProgramState(g_shaderProgram, g_pso);
	currentCmdList.GFXSetPrimitiveTopology(cfc::gpu_primitive_type::TriangleList);
	currentCmdList.GFXSetVertexBuffer(0, g_vb, g_VertexBufferSize * sizeof(ImDrawVert) * currentFrame, sizeof(ImDrawVert), g_VertexBufferSize * sizeof(ImDrawVert));
	currentCmdList.GFXSetIndexBuffer(g_ib, g_IndexBufferSize * sizeof(ImDrawIdx) * currentFrame, g_IndexBufferSize * sizeof(ImDrawIdx), sizeof(ImDrawIdx) == 2 ? cfc::gpu_format_type::R16Uint : cfc::gpu_format_type::R32Uint);
	currentCmdList.GFXSetRootParameterCBV(0, g_constantBuffer, CB_ALIGNMENT_IN_BYTES * currentFrame);

	currentCmdList.SetDescriptorHeap(g_descriptorHeap);
	currentCmdList.GFXSetDescriptorTableCbvSrvUav(1, 0); // bound location for font texture

	// TODO: might need to set blend factor to 0,0,0,0

    // Render command lists
    int vtx_offset = 0;
    int idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
				currentCmdList.GFXSetScissorRects(cfc::gpu_rectangle((long)pcmd->ClipRect.x, (long)pcmd->ClipRect.y, (long)pcmd->ClipRect.z, (long)pcmd->ClipRect.w));

				currentCmdList.GFXDrawIndexedInstanced(pcmd->ElemCount, 1, idx_offset, vtx_offset, 0);
            }
            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.size();
    }

	currentCmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(g_gfx->GetBackbufferRTResource(), cfc::gpu_resourcestate::RenderTarget, cfc::gpu_resourcestate::Present));

	currentCmdList.Close();

	g_gfx->ExecuteCommandLists(currentCmdList.GetIndex());
}

static void ImGui_ImplGfx_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Upload texture to graphics system
	{
		g_fontTexture = g_resourceStream->AddTexture(cfc::gfx_texture_creation_desc(pixels, cfc::gpu_format_type::Rgba8Unorm, width, height));
		g_resourceStream->Flush();
	}

	g_descriptorHeap->SetSRVTexture(0, g_fontTexture, cfc::gpu_format_type::Rgba8Unorm);

    // Store our identifier
    io.Fonts->TexID = nullptr;
}

bool    ImGui_ImplGfx_CreateDeviceObjects()
{
	stl_assert(g_gfx);

    if (g_isInitialized)
        ImGui_ImplGfx_InvalidateDeviceObjects();

	g_commandLists.resize(g_gfx->GetBackbufferFrameQuantity());
	for (u32 i = 0; i < g_gfx->GetBackbufferFrameQuantity(); ++i)
		g_commandLists[i] = g_gfx->GetCommandList(g_gfx->AddCommandList());

	g_descriptorHeap = g_gfx->GetDescriptorHeap(g_gfx->AddDescriptorHeap());

	g_resourceStream = g_gfx->GetResourceStream(g_gfx->AddResourceStream());

    // By using D3DCompile() from <d3dcompiler.h> / d3dcompiler.lib, we introduce a dependency to a given version of d3dcompiler_XX.dll (see D3DCOMPILER_DLL_A)
    // If you would like to use this DX11 sample code but remove this dependency you can: 
    //  1) compile once, save the compiled shader blobs into a file or source code and pass them to CreateVertexShader()/CreatePixelShader() [prefered solution]
    //  2) use code to detect any version of the DLL and grab a pointer to D3DCompile from the DLL. 
    // See https://github.com/ocornut/imgui/pull/638 for sources and details.

    // Create the vertex shader
	{
		static const char* vertexShader =
			"cbuffer r_vertexBuffer : register(b0) \
            {\
            float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
            float2 pos : POSITION;\
            float2 uv  : TEXCOORD0;\
            uint col : COLOR0;\
            };\
            \
            struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
			float colorFromUintChannel(uint color, uint channelID)\
			{\
				return (float)((color >> (channelID * 8)) & 255) / 255.0f;\
			}\
            PS_INPUT main(VS_INPUT input)\
            {\
            PS_INPUT output;\
            output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
            output.col = float4(colorFromUintChannel(input.col,0),colorFromUintChannel(input.col,1),colorFromUintChannel(input.col,2),colorFromUintChannel(input.col,3));\
            output.uv  = input.uv;\
            return output;\
            }";

		g_vs = g_gfx->AddShaderFromMemory(vertexShader, strlen(vertexShader));

		stl_assert(g_vs != cfc::invalid_index);

		// note that the size of the constant buffer needs to be 256 bytes or bigger
		g_constantBuffer = g_resourceStream->AddDynamicResource(cfc::gfx_resource_type::ConstantBuffer, CB_ALIGNMENT_IN_BYTES * g_gfx->GetBackbufferFrameQuantity(), true, false);
    }

	g_resourceStream->Flush();

    // Create the pixel shader
    {
        static const char* pixelShader =
            "struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            SamplerState r_linear_wrap : register(s1);\
            Texture2D<float4> texture0 : register(t2);\
            \
            float4 main(PS_INPUT input) : SV_Target\
            {\
            float4 out_col = input.col * texture0.Sample(r_linear_wrap, input.uv); \
            return out_col; \
            }";

		g_ps = g_gfx->AddShaderFromMemory(pixelShader, strlen(pixelShader), "main", "ps_5_0");

		stl_assert(g_ps != cfc::invalid_index);
    }

	g_shaderProgram = g_gfx->AddGraphicsProgram(g_vs, g_ps);

	// create pipeline state object
	{
		cfc::gfx_gfxprogram_desc pso;
		pso.Pipeline.RTVFormats[0] = g_gfx->GetBackbufferRTVFormat();
		pso.Pipeline.NumRenderTargets = 1;

		pso.Pipeline.PrimitiveTopology = cfc::gpu_primitivetopology_type::Triangle;

		// Create the blending setup
		{
			pso.Pipeline.BlendState.AlphaToCoverageEnable = true;
			pso.Pipeline.BlendState.RenderTarget[0].BlendEnabled = true;
			pso.Pipeline.BlendState.RenderTarget[0].SrcBlend = cfc::gpu_blendmode_type::SrcAlpha;
			pso.Pipeline.BlendState.RenderTarget[0].DstBlend = cfc::gpu_blendmode_type::InvSrcAlpha;
			pso.Pipeline.BlendState.RenderTarget[0].BlendOp = cfc::gpu_blendop_type::Add;
			pso.Pipeline.BlendState.RenderTarget[0].SrcBlendAlpha = cfc::gpu_blendmode_type::InvSrcAlpha;
			pso.Pipeline.BlendState.RenderTarget[0].DstBlendAlpha = cfc::gpu_blendmode_type::Zero;
			pso.Pipeline.BlendState.RenderTarget[0].BlendOpAlpha = cfc::gpu_blendop_type::Add;
		}

		// Create the rasterizer state
		{
			pso.Pipeline.RasterizerState.FillMode = cfc::gpu_graphicspipelinestate_desc::rasterizer_desc::FillSolid;
			pso.Pipeline.RasterizerState.CullMode = cfc::gpu_graphicspipelinestate_desc::rasterizer_desc::CullNone;
			pso.Pipeline.RasterizerState.DepthClipEnable = true;
		}

		// Create depth-stencil State
		{
			pso.Pipeline.DepthStencilState.DepthEnable = false;
			pso.Pipeline.DepthStencilState.StencilEnable = false;
		}

		g_pso = g_gfx->AddGraphicsProgramPipelineState(g_shaderProgram, pso);
	}

    ImGui_ImplGfx_CreateFontsTexture();

	g_resourceStream->WaitForFinish();

	g_isInitialized = true;

    return true;
}

void    ImGui_ImplGfx_InvalidateDeviceObjects()
{
	stl_assert(g_gfx);

	g_isInitialized = false; 

	g_gfx->RemoveResource(g_vb);
	g_vb = cfc::invalid_index;

	g_gfx->RemoveResource(g_ib);
	g_ib = cfc::invalid_index;

	g_gfx->RemoveGraphicsProgram(g_shaderProgram);
	g_shaderProgram = cfc::invalid_index;

	g_gfx->RemoveShader(g_vs);
	g_vs = cfc::invalid_index;

	g_gfx->RemoveShader(g_ps);
	g_ps = cfc::invalid_index;

	g_gfx->RemoveResource(g_constantBuffer);
	g_constantBuffer = cfc::invalid_index;

	g_gfx->RemoveResource(g_fontTexture);
	g_fontTexture = cfc::invalid_index;

	g_gfx->RemoveDescriptorHeap(g_descriptorHeap->GetIndex());
	g_descriptorHeap = nullptr;

	g_gfx->RemoveResourceStream(g_resourceStream->GetIndex());
	g_resourceStream = nullptr;

	for (u32 i = 0; i < g_commandLists.size(); ++i)
		g_gfx->RemoveCommandList(g_commandLists[i]->GetIndex());
	g_commandLists.resize(0);
}

bool    ImGui_ImplGfx_Init(cfc::context* context, cfc::gfx* gfx)
{
	stl_assert(context);
	stl_assert(gfx);

	g_context = context;
	g_gfx = gfx;

    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond))
        return false;
    if (!QueryPerformanceCounter((LARGE_INTEGER *)&g_Time))
        return false;

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    io.RenderDrawListsFn = ImGui_ImplGfx_RenderDrawLists;  // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.ImeWindowHandle = (HWND)context->Window->GetPlatformSpecificHandle();

	evKeyDown = [&io](cfc::window::eventData ev) { if (ev.keyCode < 256) io.KeysDown[ev.keyCode] = 1; };
	context->Window->OnKeyDown += evKeyDown;

	evKeyUp = [&io](cfc::window::eventData ev) { if (ev.keyCode < 256) io.KeysDown[ev.keyCode] = 0; };
	context->Window->OnKeyUp += evKeyUp;

	evCursorUp = [&io](cfc::window::eventData ev) 
	{
		if (ev.button > cfc::window::cursorButton::Unknown &&
			ev.button <= cfc::window::cursorButton::Middle)
		io.MouseDown[(u32)ev.button - 1] = false; 
	};
	context->Window->OnCursorUp += evCursorUp;

	evCursorDown = [&io](cfc::window::eventData ev) 
	{
		if(ev.button > cfc::window::cursorButton::Unknown &&
			ev.button <= cfc::window::cursorButton::Middle)
		io.MouseDown[(u32)ev.button - 1] = true; 
	};
	context->Window->OnCursorDown += evCursorDown;

	evCursorScroll = [&io](cfc::window::eventData ev) {  io.MouseWheel = ev.scroll; };
	context->Window->OnCursorScroll += evCursorScroll;

	evCursorMove = [&io](cfc::window::eventData ev) { io.MousePos.x = ev.x; io.MousePos.y = ev.y; };
	context->Window->OnCursorMove += evCursorMove;

	/* TODO: support AddInputCharacter
		case WM_CHAR:
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
			return true;
			*/

    return true;
}

void ImGui_ImplGfx_Shutdown()
{
    ImGui_ImplGfx_InvalidateDeviceObjects();
    ImGui::Shutdown();
    
	g_gfx = nullptr;
	g_context = nullptr;
}

void ImGui_ImplGfx_NewFrame()
{
    if (!g_isInitialized)
        ImGui_ImplGfx_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2((float)(g_gfx->GetBackbufferWidth()), (float)(g_gfx->GetBackbufferHeight()));

    // Setup time step
    INT64 current_time;
    QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
    io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;
    // io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
    // io.MousePos : filled by WM_MOUSEMOVE events
    // io.MouseDown : filled by WM_*BUTTON* events
    // io.MouseWheel : filled by WM_MOUSEWHEEL events

    // Hide OS mouse cursor if ImGui is drawing it
    SetCursor(io.MouseDrawCursor ? NULL : LoadCursor(NULL, IDC_ARROW));

    // Start the frame
    ImGui::NewFrame();
}
