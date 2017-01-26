// engine includes
#include <cfc/base.h>

#include <dependencies/imgui/imgui.h>
#include <cfc/gpu/renderer_imgui.h>

#include <cfc/stl/stl_unique_ptr.hpp>
#include <cfc/platform/platform_win32.hpp>
#include <cfc/gpu/gpu_d3d12.h>
#include <cfc/gpu/gfx_d3d12.h>

#include <cfc/stl/stl_threading.hpp>

// local includes
#include "scene.h"
#include "camera.h"


// defines
#define NUM_BACK_BUFFER_FRAMES 2
#define CB_ALIGNMENT_IN_BYTES 256

#define CAMERA_SPEED_MULTIPLIER 10.0f

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))


// [[ CODE ]]
struct gfx_state
{
	gfx_state() : gfx(d12_gfx) {}

private:
	enum cameras
	{
		Camera1,
		Camera2,
		Camera3,
		Camera4,
		FlyThroughCamera,
		FreeCamera,
		Count,
	};

public:
	cfc::context* context;
	cfc::gfx& gfx;
	cfc::gfx_dx12 d12_gfx;
	cfc::gfx_resource_stream* gfxResources = nullptr;
	cfc::gfx_command_list* gfxCmdLists[8];

	cfc::window::event evResize;

	// data and resources
	scene scene;
	scene::OcclusionTypes occlusionType = scene::OcclusionTypes::Gpu;

	stl_thread m_thLoading;
	bool m_sceneLoaded = false;

	// camera info
	u32 m_currentCamera = cameras::Camera1;
	camera m_cameras[cameras::Count];
	cfc::math::quatf m_cameraRotations[cameras::Count];
	cfc::math::vector3f m_cameraPositions[cameras::Count];

	cfc::math::matrix4f m_inversePrevViewMatrix;

	usize m_resViewStateConstantBuffer = cfc::invalid_index;
	usize m_resPrevViewStateConstantBuffer = cfc::invalid_index;

	f32 m_frameTimeReadOnly = 0.0f;
	f32 m_perfCaptureTimer = 0.0f;

	bool m_wireFrame = false;
	bool m_useVertexShaderAsCompute = false;
	bool m_doDownsampleAfterReproject = true;
	bool m_gpuOcclusionCullingEnabled = true;
	bool m_allowForPeriodicPerformanceCaptures = false;
	scene::DebugRenderMode m_debugRenderMode = scene::NoDebugRender;

public:
	void init(cfc::context& ctx)
	{
		context = &ctx;

		// due to the current order of operations, OnResize event for this app needs to be registered before gfx init, gfx _resize needs to run before app to keep gfx info correct for current resize
		evResize = [this](cfc::window::eventData ev) { gfx.WaitForGpu(); resize(); };
		context->Window->OnResize += evResize;

		// init device
		gfx.Init(ctx, 0, NUM_BACK_BUFFER_FRAMES, cfc::gpu_swapimage_type::Rgba8UnormSrgb, cfc::gpu_format_type::D24UnormS8Uint);
	
		// create command lists
		for (usize i = 0; i < gfx.GetBackbufferFrameQuantity(); i++)
			gfxCmdLists[i] = gfx.GetCommandList(gfx.AddCommandList());

		ImGui_ImplGfx_Init(context, &gfx);

		// create resource stream
		gfxResources = gfx.GetResourceStream(gfx.AddResourceStream());

		m_resViewStateConstantBuffer = gfxResources->AddDynamicResource(cfc::gfx_resource_type::ConstantBuffer, CB_ALIGNMENT_IN_BYTES * gfx.GetBackbufferFrameQuantity(), false);
		m_resPrevViewStateConstantBuffer = gfxResources->AddDynamicResource(cfc::gfx_resource_type::ConstantBuffer, CB_ALIGNMENT_IN_BYTES * gfx.GetBackbufferFrameQuantity(), false);

		cfc::math::vector3f cameraStartPositions[cameras::Count] = {
			cfc::math::vector3f(-9.0f,  0.2f, -5.0f) ,
			cfc::math::vector3f(-9.0f,  0.2f, -5.0f) ,
			cfc::math::vector3f(-15.0f,  6.4f, -5.8f) ,
			cfc::math::vector3f(-3.8f, 15.2f, -1.86f) ,
			cfc::math::vector3f(-2.2f,  1.3f,  0.0f)  ,
			cfc::math::vector3f(-9.0f,  0.2f, -5.0f) };

		cfc::math::quatf cameraRotations[cameras::Count] = {
			cfc::math::quatf( 0.0f,  -0.7f, 0.0f, -0.7f) ,
			cfc::math::quatf( 0.0f,  -0.7f,  0.0f, 0.7f) ,
			cfc::math::quatf(-0.3f,  -0.4f,  0.1f, 0.9f) ,
			cfc::math::quatf(-0.7f,  -0.1f,  0.1f, 0.7f) ,
			cfc::math::quatf(0.0f,  -0.7f,  0.0f, 0.7f)  ,
			cfc::math::quatf(0.0f,  -0.7f,  0.0f, 0.7f) };

		for (u32 i = 0; i < cameras::Count; ++i)
		{
			m_cameraPositions[i] = cameraStartPositions[i];
			m_cameraRotations[i] = cameraRotations[i];
			m_cameras[i] = camera(55.0f, 36.0f, 24.0f, cameraStartPositions[i]);
			m_cameras[i].LookAt(cameraStartPositions[i] + m_cameraRotations[i].GetLocalZ());
		}

		gfxResources->Flush();

		gfxResources->WaitForFinish();

		m_thLoading = stl_thread(
		[this]() {
#if DRAW_SPONZA
			this->scene.Load(context, gfx, "crytek_sponza/sponza.obj");
#else
			this->scene.Load(context, gfx, "cube/cube.obj");
#endif
			m_sceneLoaded = true;
		});

		m_thLoading.detach();
	}

	void release()
	{
		gfx.RemoveResource(m_resViewStateConstantBuffer);
		gfx.RemoveResource(m_resPrevViewStateConstantBuffer);

		scene.Unload(gfx);

		ImGui_ImplGfx_Shutdown();
	}

	void resize()
	{
		ImGui_ImplGfx_CreateDeviceObjects();

		scene.Resize(gfx);
	}

	void update(const f32 deltaTimeSeconds)
	{
		static f32 g_tempMovementTime = 0;
		g_tempMovementTime += deltaTimeSeconds;

		view_state& viewState = m_cameras[m_currentCamera].GetViewState();
		m_inversePrevViewMatrix = (viewState.ProjectionMatrix * viewState.ViewMatrix).Inverted();
		
		// update camera (constant buffer) through cpu->gpu upload buffer
		float speedMul = context->Window->IsKeyDown(' ') ? CAMERA_SPEED_MULTIPLIER : 1.0f;
		speedMul *= deltaTimeSeconds;

		{
			cfc::math::vector3f camPosition = m_cameras[m_currentCamera].GetPosition();
			camPosition += m_cameraRotations[m_currentCamera].GetLocalX() * (context->Window->IsKeyDown('A') ? speedMul : (context->Window->IsKeyDown('D') ? -speedMul : 0.0f));
			camPosition += m_cameraRotations[m_currentCamera].GetLocalZ() * (context->Window->IsKeyDown('W') ? speedMul : (context->Window->IsKeyDown('S') ? -speedMul : 0.0f));
			camPosition += m_cameraRotations[m_currentCamera].GetLocalY() *  (context->Window->IsKeyDown('E') ? speedMul : (context->Window->IsKeyDown('Q') ? -speedMul : 0.0f));

			if (context->Window->IsCursorDown(2, 0))
			{
				m_cameraRotations[m_currentCamera] *= cfc::math::quatf().CreateAxisAngleQuat(cfc::math::vector3f(0.0f, 1.0f, 0.0f), context->Window->GetCursorDeltaX()*0.01f);
				m_cameraRotations[m_currentCamera] *= cfc::math::quatf().CreateAxisAngleQuat(m_cameraRotations[m_currentCamera].GetLocalX(), context->Window->GetCursorDeltaY()*-0.01f);
				m_cameraRotations[m_currentCamera].NormalizeQuat();
			}
			m_cameras[m_currentCamera].SetPosition(camPosition);
			m_cameras[m_currentCamera].LookAt(camPosition + m_cameraRotations[m_currentCamera].GetLocalZ(), m_cameraRotations[m_currentCamera].GetLocalY());
		}
		
		// update fly camera
		float x = sin(g_tempMovementTime) * 6.0;
		float y = 2.0 - ((x * x) / 12.0);
		m_cameras[cameras::FlyThroughCamera].SetPosition(m_cameraPositions[cameras::FlyThroughCamera] + cfc::math::vector3f(x, y, 0));
		cfc::math::vector3f camPosition = m_cameras[cameras::FlyThroughCamera].GetPosition();
		m_cameras[cameras::FlyThroughCamera].LookAt(camPosition + m_cameraRotations[cameras::FlyThroughCamera].GetLocalZ(), m_cameraRotations[cameras::FlyThroughCamera].GetLocalY());

		scene.AllowWireFrame(m_wireFrame);

		scene.AllowComputeAsVertexShader(m_useVertexShaderAsCompute);

		scene.AllowDownsampleAfterReproject(m_doDownsampleAfterReproject);

		scene.SetDebugRenderMode(m_debugRenderMode);

		occlusionType = m_gpuOcclusionCullingEnabled ? scene::OcclusionTypes::Gpu : scene::OcclusionTypes::None;

		// potentially do performance capture
		if (m_sceneLoaded && m_allowForPeriodicPerformanceCaptures)
		{
			// collect min, max and average
			static float minVals[9] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
			static float maxVals[9] = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
			static float average[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			static float frameCounter = 1;

			stl_vector<cfc::gfx_gpu_timer_query> timerQueries;
			scene.GatherFrameTimerQueries(gfx, occlusionType, timerQueries);
			for (u32 i = 0; i < timerQueries.size(); ++i)
			{
				minVals[i] = min(minVals[i], (f32)gfx.GetTimerQueryResultInMS(timerQueries[i]));
				maxVals[i] = max(maxVals[i], (f32)gfx.GetTimerQueryResultInMS(timerQueries[i]));
				average[i] += (f32)gfx.GetTimerQueryResultInMS(timerQueries[i]);
			}

			if (m_perfCaptureTimer >= 2.0f)
			{
				printf("Total Draw Time & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms\\\\\n", minVals[0], maxVals[0], average[0] / frameCounter, 0.0f, 0.0f, 0.0f);
				
				if (timerQueries.size() > 1)
				{
					printf("Clear UAV & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms\\\\\n", minVals[1], maxVals[1], average[1] / frameCounter, 0.0f, 0.0f, 0.0f);
					printf("Reproject & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms\\\\\n", minVals[2], maxVals[2], average[2] / frameCounter, 0.0f, 0.0f, 0.0f);
					printf("Downsample Reproject & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms\\\\\n", minVals[3], maxVals[3], average[3] / frameCounter, 0.0f, 0.0f, 0.0f);
					printf("Copy Reproject to Depth & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms\\\\\n", minVals[4], maxVals[4], average[4] / frameCounter, 0.0f, 0.0f, 0.0f);
					printf("Rasterize Occludees & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms\\\\\n", minVals[5], maxVals[5], average[5] / frameCounter, 0.0f, 0.0f, 0.0f);
					printf("Clear Append Buffer & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms\\\\\n", minVals[6], maxVals[6], average[6] / frameCounter, 0.0f, 0.0f, 0.0f);
					printf("Gather Visible Objects & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms\\\\\n", minVals[7], maxVals[7], average[7] / frameCounter, 0.0f, 0.0f, 0.0f);
					printf("Draw Visible Objects & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms & %2.3fms\\\\\n", minVals[8], maxVals[8], average[8] / frameCounter, 0.0f, 0.0f, 0.0f);
				}
				printf("\n");

				m_perfCaptureTimer = 0.0f;
				for (u32 i = 0; i < 9; ++i)
				{
					minVals[i] = FLT_MAX;
					maxVals[i] = -FLT_MAX;
					average[i] = 0;
				}
			}

			m_perfCaptureTimer += deltaTimeSeconds;

		}

		m_frameTimeReadOnly = deltaTimeSeconds;
	}

	void render()
	{
		const usize frameIndex = gfx.GetBackbufferFrameIndex();

		view_state& viewState = m_cameras[m_currentCamera].GetViewState();
		viewState.ScreenWidth = gfx.GetBackbufferWidth();
		viewState.ScreenHeight = gfx.GetBackbufferHeight();
		gfxResources->UpdateDynamicResource(m_resViewStateConstantBuffer, sizeof(view_state), &viewState, CB_ALIGNMENT_IN_BYTES * frameIndex);

		gfxResources->UpdateDynamicResource(m_resPrevViewStateConstantBuffer, sizeof(cfc::math::matrix4f), &m_inversePrevViewMatrix, CB_ALIGNMENT_IN_BYTES * frameIndex);
		gfxResources->Flush();

		// get current command list
		cfc::gfx_command_list& currentCmdList = *gfxCmdLists[frameIndex];
		currentCmdList.Reset();

		currentCmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(gfx.GetBackbufferRTResource(), cfc::gpu_resourcestate::Present, cfc::gpu_resourcestate::RenderTarget));

		if (m_sceneLoaded)
			scene.Render(gfx, currentCmdList, occlusionType, m_resViewStateConstantBuffer, m_resPrevViewStateConstantBuffer, viewState);

		currentCmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(gfx.GetBackbufferRTResource(), cfc::gpu_resourcestate::RenderTarget, cfc::gpu_resourcestate::Present));

		currentCmdList.Close();

		gfx.ExecuteCommandLists(currentCmdList.GetIndex());

		renderUI();

		if (m_sceneLoaded)
		{
			// resolves current frame queries
			gfx.ResolveTimerQueries();

			// readback of (currentFrame - (TIMER_QUERIES_FRAMES_DELAY - 1))
			gfx.ReadBackTimerQueryResults();
		}

		gfx.Present(0, 0);
	}

	void renderUI()
	{
		// render UI
		ImGui_ImplGfx_NewFrame();

		if (ImGui::CollapsingHeader("Help"))
		{
			ImGui::TextWrapped("This is the implementation of GPU occlusion culling using execute indirect by Juul Joosten.");
			{
				ImGui::BulletText("Double-click on debug bar to collapse window.");
				ImGui::BulletText("Hold right mouse button and drag to rotate the camera");
				ImGui::BulletText("Hold WSAD to move along the cameras local axis");
				ImGui::BulletText("Hold Space to strafe");
				ImGui::BulletText("Press Escape to quit");
				ImGui::BulletText("DONT toggle the vertex as compute shader on AMD");
				ImGui::BulletText("Use ALT+TAB to toggle fullscreen mode");
				ImGui::TextWrapped("Note that wireframe flickering is caused from the unsorted indirect draw calls");
			}
		}

		if (!m_sceneLoaded)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Loading scene (%s)..", scene.GetStatus().c_str());
		}

		float fasterThan60hz = (1.0f / m_frameTimeReadOnly) <= 0.016 ? 1.0 : 0.0;
		float slowerThan60hz = 1.0 - fasterThan60hz;
		ImGui::TextColored(ImVec4(fasterThan60hz, slowerThan60hz, 0, 1), "FPS %f fps   DT %f ms \n", 1.0f / m_frameTimeReadOnly, m_frameTimeReadOnly * 1000.0f);

		if (m_sceneLoaded)
		{
			stl_vector<cfc::gfx_gpu_timer_query> timerQueries;
			scene.GatherFrameTimerQueries(gfx, occlusionType, timerQueries);

			float indirectRenderingEnabled = m_gpuOcclusionCullingEnabled ? 1.0 : 0.0;
			float indirectRenderingDisabled = 1.0 - indirectRenderingEnabled;
			ImGui::TextColored(ImVec4(indirectRenderingDisabled, indirectRenderingEnabled, 0, 1), "Time: %f ms Desc: %s \n", (f32)gfx.GetTimerQueryResultInMS(timerQueries[0]), timerQueries[0].GetDescription());
			if (ImGui::CollapsingHeader("In-Depth Timings"))
			{
				for (u32 i = 1; i < timerQueries.size(); ++i)
					ImGui::Text("Time: %f ms Desc: %s \n", (f32)gfx.GetTimerQueryResultInMS(timerQueries[i]), timerQueries[i].GetDescription());
			}
		}

		ImGui::Checkbox("Render Using Execute Indirect (click here)", &m_gpuOcclusionCullingEnabled);
		ImGui::Checkbox("Toggle wireframe (click here)", &m_wireFrame);
		ImGui::Checkbox("Toggle downsample after reproject (click here)", &m_doDownsampleAfterReproject);
		ImGui::Checkbox("Toggle vertex as compute (NO AMD SUPPORT!) (click here)", &m_useVertexShaderAsCompute);
		ImGui::Checkbox("Allow for periodic perf captures (click here)", &m_allowForPeriodicPerformanceCaptures);

		const char* cameraNames[] = { "Camera 1. 'Lions Head'", "Camera 2. 'Into Sponza'", "Camera 3. 'Edge Overview Camera'", "Camera 4. 'Top Down Camera'", "Camera 5. 'Auto Fly Camera'", "Camera 6. 'User Movement Camera'" };
		if (ImGui::Button("Camera View Select.."))
			ImGui::OpenPopup("cameraselect");
		ImGui::SameLine();
		ImGui::Text(cameraNames[m_currentCamera]);
		if (ImGui::BeginPopup("cameraselect"))
		{
			ImGui::Text("Cameras");
			ImGui::Separator();
			for (int i = 0; i < IM_ARRAYSIZE(cameraNames); i++)
				if (ImGui::Selectable(cameraNames[i]))
					m_currentCamera = i;
			ImGui::EndPopup();
		}

		const char* debugViewNames[] = { "None", "Previous Depth Buffer", "Reprojected Depth Buffer", "Downsampled Reprojected Depth Buffer", "All" };
		if (ImGui::Button("Debug View Select.."))
			ImGui::OpenPopup("debugselect");
		ImGui::SameLine();
		ImGui::Text(debugViewNames[m_debugRenderMode]);
		if (ImGui::BeginPopup("debugselect"))
		{
			ImGui::Text("Debug Views");
			ImGui::Separator();
			for (int i = 0; i < IM_ARRAYSIZE(debugViewNames); i++)
				if (ImGui::Selectable(debugViewNames[i]))
					m_debugRenderMode = (scene::DebugRenderMode)i;
			ImGui::EndPopup();
		}

		ImGui::Render();
	}
};

bool g_requestStop = false;
cfc::window::event escape;
void app_main(cfc::context* bcontext)
{
	cfc::context context(*bcontext);

	// * Initialize core components with context dependencies.
	char wndClass[20];
	sprintf(wndClass, "CfcClass%d", rand());

	// * Initialize window & graphics
	cfc::platform::win32::window window(context, 1280, 720, cfc::window::fullscreenMode::ExclusiveFullscreen, "CFC - Example - Basic Execute Indirect", wndClass);
	context.Window = &window;

	// add escape as quit key
	escape = [](cfc::window::eventData ev) { if (ev.keyCode == 0x1B)g_requestStop = true; };
	context.Window->OnKeyDown += escape;

	try
	{
		stl_unique_ptr<gfx_state> gfx(new gfx_state());
		gfx->init(context);

		// * Main loop
		double prevDeltaSeconds = context.Timing->GetTimeSeconds();
		while (context.Window->IsRequestingStop() == false && g_requestStop == false)
		{
			double curDeltaSeconds = context.Timing->GetTimeSeconds();

			float delta = (float)(curDeltaSeconds - prevDeltaSeconds);

			gfx->update(delta);

			gfx->render();

			// execute list
			context.Window->Update();
			prevDeltaSeconds = curDeltaSeconds;
		}

		gfx->release();
	}
	catch (...)
	{

	}

}