#include "scene.h"

#include <dependencies/stb/stb_obj_loader.h>
#include <dependencies/stb/stb_image.h>
#include <dependencies/stb/stb_image_mipmap.h>
#include <dependencies/collision/libcollision.h>

#include <cfc/stl/stl_string.hpp>
#include <cfc/stl/stl_unique_ptr.hpp>
#include <cfc/stl/stl_string_advanced.hpp>

#include <cfc/gpu/gfx.h>
#include <cfc/gpu/gfx_d3d12.h>
#include <cfc/gpu/gpu_d3d12.h>


#include "camera.h"

#define REQUEST_RGBA_4_COMPONENTS 4

#define CB_ALIGNMENT_IN_BYTES 256


static const u32 g_numVerticesCube = 8;
static const u32 g_numIndicesCube = 36;
static const u32 g_indexBufferDataCube[] = { 0, 1, 2, 2, 1, 3,		// front face
											 4, 0, 6, 6, 0, 2,		// left face
											 5, 4, 7, 7, 4, 6,		// back face
											 1, 5, 3, 3, 5, 7,		// right face
											 4, 5, 0, 0, 5, 1,		// top face
											 2, 3, 6, 6, 3, 7 };		// bottom face


struct vert_pos_uv
{
	float X, Y, Z;
	float U, V;
};

struct vert_pos
{
	float X, Y, Z;
};

struct indirectDrawOpaqueArgs
{
	cfc::gpu_dx12_cmdlist_indirect_api::dx12_indirect_command_descriptors::ICIndexBufferViewArgs IBV;
	cfc::gpu_dx12_cmdlist_indirect_api::dx12_indirect_command_descriptors::ICVertexBufferViewArgs VBV;
	u32 ModelMatrixIndex;
	u32 AlbedoTextureDescriptorTableIdx;
	cfc::gpu_dx12_cmdlist_indirect_api::dx12_indirect_command_descriptors::ICDrawIndexedInstancedArgs Draw;
	u32 _padding;
};



void generateCubePositions(vert_pos inOut[8])
{
	// build unit cube (size xyz=1, center xyz=0)
	for (u32 i = 0; i < 8; ++i)
	{
		// generate xyz offsets by bit field exploitation
		const float xOffset = (float)((i & 1) >> 0); // 01010101...
		const float yOffset = (float)((i & 2) >> 1); // 00110011...
		const float zOffset = (float)((i & 4) >> 2); // 00001111...

		inOut[i].X = -0.5f + xOffset;
		inOut[i].Y = -0.5f + yOffset;
		inOut[i].Z = -0.5f + zOffset;
	}
}

float ucharColorChannelToFloat(unsigned char x)
{
	return (float)x / 255.0f;
}

unsigned char floatColorChannelToUChar(float x)
{
	return (unsigned char)(x * 255.0f);
}

void scene::Load(cfc::context* const context, cfc::gfx& gfx, stl_string sceneFile)
{
	cfc::gfx_resource_stream* gfxResourceStream = gfx.GetResourceStream(gfx.AddResourceStream());

	m_opaqueRenderingDescHeap = gfx.GetDescriptorHeap(gfx.AddDescriptorHeap());
	m_depthBufferDescHeap = gfx.GetDescriptorHeap(gfx.AddDescriptorHeap());

	char resourceNameBuffer[128];

	// DX12 INTEROP
	cfc::gfx_dx12& dx12Gfx = static_cast<cfc::gfx_dx12&>(gfx);
	cfc::gpu_dx12_context& dx12Context = *reinterpret_cast<cfc::gpu_dx12_context*>(dx12Gfx.DX12_GetContext());

	// load render passes
	setStatus(stl_string_advanced::sprintf("Loading render passes."));
	m_downSampleReprojectedDepthBufferCmp.Load(context, gfx, 32, 32);
	m_reprojectDepthBufferCmp.Load(context, gfx, 32, 32);
	m_copyReprojectDepthBuffer.Load(context, gfx);
	m_renderVisibilityGfx.Load(context, gfx);
	m_collectVisibleDrawCallsCmp.Load(context, gfx, 1024);
	m_collectVisibleDrawCallsCmpLowOverhead.Load(context, gfx);
	m_renderOpaqueGfx.Load(context, gfx);
	m_debugFullScreenTexQuad.Load(context, gfx);
	m_debugOpaqueWireFrameGfx.Load(context, gfx);

	// timer query groups indirect
	m_timerQueryIndirectDrawFrame.resize(gfx.GetTimerQueryFrameDelayQuantity());
	m_timerQueryClearUav.resize(gfx.GetTimerQueryFrameDelayQuantity());
	m_timerQueryReprojectDepth.resize(gfx.GetTimerQueryFrameDelayQuantity());
	m_timerQueryDownSampleReprojectedDepth.resize(gfx.GetTimerQueryFrameDelayQuantity());
	m_timerQueryCopyUAVToDepth.resize(gfx.GetTimerQueryFrameDelayQuantity());
	m_timerQueryDrawAABBs.resize(gfx.GetTimerQueryFrameDelayQuantity());
	m_timerQueryClearAppendBufferPass.resize(gfx.GetTimerQueryFrameDelayQuantity());
	m_timerQueryAquireVisibleObjects.resize(gfx.GetTimerQueryFrameDelayQuantity());
	m_timerQueryIndirectDraw.resize(gfx.GetTimerQueryFrameDelayQuantity());

	// timer query groups direct
	m_timerQueryDirectDraw.resize(gfx.GetTimerQueryFrameDelayQuantity());

	// get base path of obj file for material loading
	stl_string basePath = "";	

	// NOTE: + 1 append to lastIndexOfFolderDivide makes sure to include '/' character as folder divide
	usize lastIndexOfFolderDivide = sceneFile.find_last_of('/');
	if (lastIndexOfFolderDivide != std::string::npos)
		basePath = sceneFile.substr(0, lastIndexOfFolderDivide + 1);

	// load meshes and materials for visual fidelity
	setStatus(stl_string_advanced::sprintf("Loading OBJ (%s)", sceneFile.c_str()));
	stl_vector<tinyobj::shape_t> meshes;
	stl_vector<tinyobj::material_t> materials;
	stl_assert(tinyobj::LoadObj(meshes, materials, sceneFile.c_str(), basePath.c_str()) == "");

	u32 gridSize = GRID_SIZE;
	u32 numLoadedMeshes = (u32)meshes.size();
	m_maxNumMeshesToRender = numLoadedMeshes * gridSize * gridSize;
	m_vertexBuffers.resize(m_maxNumMeshesToRender);
	m_indexBuffers.resize(m_maxNumMeshesToRender);
	m_materialIds.resize(m_maxNumMeshesToRender);
	m_modelMatrices.resize(m_maxNumMeshesToRender);
	m_aabbs.resize(m_maxNumMeshesToRender);
	m_final_aabbs.resize(m_maxNumMeshesToRender);
	for (u32 y = 0; y < gridSize; ++y)
	{
		for (u32 x = 0; x < gridSize; ++x)
		{
			for (u32 i = 0; i < numLoadedMeshes; ++i)
			{
				setStatus(stl_string_advanced::sprintf("Processing mesh (%d/%d/%d).", x, y, i));
				const u32 gridIndex = y * gridSize * numLoadedMeshes + x * numLoadedMeshes + i;

				tinyobj::mesh_t& mesh = meshes[i].mesh;

				const usize numVertices = mesh.positions.size() / 3;

				// interleave vertex data (positions, uvs)
				stl_vector<vert_pos_uv> vertices(numVertices);
				for (usize v = 0; v < numVertices; ++v)
				{
					vertices[v].X = mesh.positions[v * 3 + 0];
					vertices[v].Y = mesh.positions[v * 3 + 1];
					vertices[v].Z = mesh.positions[v * 3 + 2];

					// NOTE: V gets flipped to conform to DX UV space (bottom left 0,0)
					vertices[v].U = mesh.texcoords[v * 2 + 0];
					vertices[v].V = 1.0f - mesh.texcoords[v * 2 + 1];
				}
				m_vertexBuffers[gridIndex].SizeInBytes = (u32)(numVertices * sizeof(vert_pos_uv));
				m_vertexBuffers[gridIndex].StrideInBytes = sizeof(vert_pos_uv);
				m_vertexBuffers[gridIndex].GFXResourceIndex = gfxResourceStream->AddStaticResource(cfc::gfx_resource_type::VertexBuffer, &vertices[0], m_vertexBuffers[gridIndex].SizeInBytes);

				sprintf(resourceNameBuffer, "m_vertexBuffers[%d]", gridIndex);
				dx12Context.ResourceSetName(m_vertexBuffers[gridIndex].GFXResourceIndex, resourceNameBuffer);

				// flip winding order
				const u32 numIndices = (u32)mesh.indices.size();
				const u32 numFaces = (u32)mesh.indices.size() / 3;

				stl_vector<u32> indices(mesh.indices);
				for (usize j = 0; j < numFaces; ++j)
				{
					const usize index = j * 3;
					std::swap(indices[index], indices[index + 2]);
				}

				m_indexBuffers[gridIndex].SizeInBytes = numIndices * sizeof(u32);
				m_indexBuffers[gridIndex].NumIndices = numIndices;
				m_indexBuffers[gridIndex].GFXResourceIndex = gfxResourceStream->AddStaticResource(cfc::gfx_resource_type::IndexBuffer, &indices[0], m_indexBuffers[gridIndex].SizeInBytes);

				sprintf(resourceNameBuffer, "m_indexBuffers[%d]", gridIndex);
				dx12Context.ResourceSetName(m_indexBuffers[gridIndex].GFXResourceIndex, resourceNameBuffer);

				// we only support the first ID at the moment
				m_materialIds[gridIndex] = mesh.material_ids[0];

				// setup a simple grid model matrix offset
				memset(m_modelMatrices[gridIndex].Mat, 0, sizeof(mat4_simple));
				m_modelMatrices[gridIndex].Mat[0] = MODEL_SCALE;
				m_modelMatrices[gridIndex].Mat[5] = MODEL_SCALE;
				m_modelMatrices[gridIndex].Mat[10] = MODEL_SCALE;
				m_modelMatrices[gridIndex].Mat[12] = (-(float)gridSize * 0.5f + (float)x) * 4; //xpos
				m_modelMatrices[gridIndex].Mat[13] = 0; //ypos
				m_modelMatrices[gridIndex].Mat[14] = (-(float)gridSize * 0.5f + (float)y) * 2.5f; //zpos
				m_modelMatrices[gridIndex].Mat[15] = MODEL_SCALE;

				// generate aabbs
				// NOTE: for the maximum we cant use FLT_MIN since FLT_MIN returns the minimum positive value!
				float min[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
				float max[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

				for (u32 v = 0; v < numVertices; ++v)
				{
					const u32 index = v * 3;
					for (u32 j = 0; j < 3; ++j)
					{
						min[j] = min[j] < mesh.positions[index + j] ? min[j] : mesh.positions[index + j];
						max[j] = max[j] > mesh.positions[index + j] ? max[j] : mesh.positions[index + j];
					}
				}

				memcpy(m_aabbs[gridIndex].Min, min, sizeof(float) * 3);
				memcpy(m_aabbs[gridIndex].Max, max, sizeof(float) * 3);

				m_final_aabbs[gridIndex] = m_aabbs[gridIndex];
				collision::PrimAABB& finalAABB = (collision::PrimAABB&)m_final_aabbs[gridIndex];
				finalAABB.Transform(finalAABB, (collision::mat44f&)m_modelMatrices[gridIndex]);
			}
			gfxResourceStream->Flush();
		}
	}

	m_modelMatricesGFXResourceIndex = gfxResourceStream->AddStaticResource(cfc::gfx_resource_type::SRVBuffer, &m_modelMatrices[0], sizeof(mat4_simple) * m_maxNumMeshesToRender);
	dx12Context.ResourceSetName(m_modelMatricesGFXResourceIndex, "m_modelMatricesGFXResourceIndex");

	gfxResourceStream->Flush();

	m_albedoTextureGFXResourceIndex.reserve(materials.size() + 1);

	// generate default texture
	const u32 whiteTextureDataRGBA[4]{ 0xFF00FFFF, 0xFF00FFFF, 0xFF00FFFF, 0xFF00FFFF };
	const u32 texDescTableIndexDefault = (u32)m_albedoTextureGFXResourceIndex.size();
	m_defaultMaterial.AlbedoGFXResourceDescTableIndex = texDescTableIndexDefault;
	m_albedoTextureGFXResourceIndex.push_back(gfxResourceStream->AddTexture(cfc::gfx_texture_creation_desc(&whiteTextureDataRGBA, cfc::gpu_format_type::Rgba8UnormSrgb, 2, 2)));
	m_opaqueRenderingDescHeap->SetSRVTexture(texDescTableIndexDefault, m_albedoTextureGFXResourceIndex[texDescTableIndexDefault]);

	dx12Context.ResourceSetName(m_albedoTextureGFXResourceIndex[texDescTableIndexDefault], "default texture");

	gfxResourceStream->Flush();

	// load materials
	m_materials.resize(materials.size());
	for (u32 i = 0; i < materials.size(); ++i)
	{
		tinyobj::material_t& material = materials[i];

		stl_string albedoTexturePath = basePath + material.diffuse_texname;

		setStatus(stl_string_advanced::sprintf("Loading texture (%s).", albedoTexturePath.c_str()));

		// load albedo texture
		i32 width, height, numComponents = 0;
		stbi_uc* image = stbi_load(albedoTexturePath.c_str(), &width, &height, &numComponents, REQUEST_RGBA_4_COMPONENTS);
		m_materials[i].AlbedoGFXResourceDescTableIndex = m_defaultMaterial.AlbedoGFXResourceDescTableIndex;

		if (image == nullptr)
			continue;

		// only supply premul alpha when texture contains alpha
		if (numComponents == REQUEST_RGBA_4_COMPONENTS)
		{
			const usize numPixels = width * height;
			for (u32 x = 0; x < numPixels; ++x)
			{
				const u32 pixelIndex = x * numComponents;
				const float alpha = ucharColorChannelToFloat(image[pixelIndex + 3]);
				for (u32 c = 0; c < 3; ++c)
				{
					const float preMulColorChannel = ucharColorChannelToFloat(image[pixelIndex + c]) * alpha;
					image[pixelIndex + c] = floatColorChannelToUChar(preMulColorChannel);
				}
			}
		}

		setStatus( stl_string_advanced::sprintf("Generating mips for texture (%s).", albedoTexturePath.c_str()));

		// generate mips
		int channels = 4;
		int mipLevels = stbi_mipmap_info_quantity(width, height);
		int mipBytes = stbi_mipmap_info_bytes(width, height, channels, 0, mipLevels);
		stl_unique_ptr<unsigned char[]> nimage(new unsigned char[mipBytes]);

		// - level 0
		memcpy(&nimage[0], image, width*height * channels);

		// - level 1-n
		for (int i = 1; i < mipLevels; i++)
		{
			int w, h;
			stbi_mipmap_info_dimensions(width, height, i - 1, &w, &h);
			int offsetSource = stbi_mipmap_info_bytes(width, height, channels, 0, i-1);
			int offsetDest = offsetSource + stbi_mipmap_info_bytes(width, height, channels, i-1, 1);
			stbi_mipmap_image(&nimage[offsetSource], w, h, channels, &nimage[offsetDest], 2, 2);
		}


		const u32 texDescTableIndex = (u32)m_albedoTextureGFXResourceIndex.size();
		m_materials[i].AlbedoGFXResourceDescTableIndex = texDescTableIndex;
		m_albedoTextureGFXResourceIndex.push_back(gfxResourceStream->AddTexture(cfc::gfx_texture_creation_desc(&nimage[0], cfc::gpu_format_type::Rgba8UnormSrgb, width, height, mipLevels)));
		//m_albedoTextureGFXResourceIndex.push_back(gfxResourceStream->AddTexture(cfc::gfx_texture_creation_desc(image, cfc::gpu_format_type::Rgba8UnormSrgb, width, height, 1));
		m_opaqueRenderingDescHeap->SetSRVTexture(texDescTableIndex, m_albedoTextureGFXResourceIndex[texDescTableIndex]);

		sprintf(resourceNameBuffer, "opaque texture [%d]", i);
		dx12Context.ResourceSetName(m_albedoTextureGFXResourceIndex[texDescTableIndex], resourceNameBuffer);

		gfxResourceStream->Flush();

		free(image);
	}

	setStatus(stl_string_advanced::sprintf("Finalizing."));

	// generate visibility mesh (aabb)
	vert_pos visibilityAABBMeshVerts[g_numVerticesCube];
	generateCubePositions(visibilityAABBMeshVerts);

	m_aabbVertexBuffer.SizeInBytes = sizeof(vert_pos) * g_numVerticesCube;
	m_aabbVertexBuffer.StrideInBytes = sizeof(vert_pos);
	m_aabbVertexBuffer.GFXResourceIndex = gfxResourceStream->AddStaticResource(cfc::gfx_resource_type::VertexBuffer, visibilityAABBMeshVerts, m_aabbVertexBuffer.SizeInBytes);
	
	dx12Context.ResourceSetName(m_aabbVertexBuffer.GFXResourceIndex, "m_aabbVertexBuffer");

	m_aabbIndexBuffer.NumIndices = g_numIndicesCube;
	m_aabbIndexBuffer.SizeInBytes = g_numIndicesCube * sizeof(u32);
	m_aabbIndexBuffer.GFXResourceIndex = gfxResourceStream->AddStaticResource(cfc::gfx_resource_type::IndexBuffer, g_indexBufferDataCube, m_aabbIndexBuffer.SizeInBytes);

	dx12Context.ResourceSetName(m_aabbIndexBuffer.GFXResourceIndex, "m_aabbIndexBuffer");

	// generate aabb transform matrices
	m_aabbTransScaleMatrices.resize(m_aabbs.size());
	for (u32 i = 0; i < m_aabbs.size(); ++i)
	{
		for (u32 j = 0; j < 3; ++j)
		{
			// calculate scale
			float scale = m_aabbs[i].Max[j] - m_aabbs[i].Min[j];
			m_aabbTransScaleMatrices[i].Mat[j * 5] = scale;
			
			// calculate center position
			m_aabbTransScaleMatrices[i].Mat[12 + j] = m_aabbs[i].Min[j] + scale * 0.5f;
		}
		m_aabbTransScaleMatrices[i].Mat[15] = 1.0f;
	}
	m_aabbTransScaleMatricesGFXResourceIndex = gfxResourceStream->AddStaticResource(cfc::gfx_resource_type::SRVBuffer, &m_aabbTransScaleMatrices[0], sizeof(mat4_simple) * m_aabbTransScaleMatrices.size());

	dx12Context.ResourceSetName(m_aabbTransScaleMatricesGFXResourceIndex, "m_aabbTransScaleMatrices");

	// create visibility UAV
	stl_vector<u32> visibilityBuffer(m_aabbs.size());
	m_visibilityBufferGFXResourceIndex.resize(gfx.GetBackbufferFrameQuantity());
	for (u32 i = 0; i < gfx.GetBackbufferFrameQuantity(); ++i)
	{
		m_visibilityBufferGFXResourceIndex[i] = gfxResourceStream->AddStaticResource(cfc::gfx_resource_type::UAVBuffer, &visibilityBuffer[0], sizeof(u32) * visibilityBuffer.size());

		sprintf(resourceNameBuffer, "m_visibilityBufferGFXResourceIndex[%d]", i);
		dx12Context.ResourceSetName(m_visibilityBufferGFXResourceIndex[i], resourceNameBuffer);
	}
	
	gfxResourceStream->Flush();

	// DX12 INTEROP
	usize rootSignatureIdx = dx12Gfx.DX12_GetRootSignatureIdxFromProgram(m_renderOpaqueGfx.GetShaderProgram());
	
	m_opaqueIndirectCmdList = new cfc::gpu_dx12_cmdlist_indirect_api(dx12Context);
	m_opaqueIndirectCmdList->ICIASetIndexBuffer();
	m_opaqueIndirectCmdList->ICIASetVertexBuffers(0, 1);
	m_opaqueIndirectCmdList->ICSetRoot32BitConstants(2, 2, 0);
	m_opaqueIndirectCmdList->ICDrawIndexedInstanced();

	// note since we are using RootConstants in the indirect command list, we need to set a root signature.
	bool indirectTemplateCompiled = m_opaqueIndirectCmdList->CompileIC(rootSignatureIdx);

	// create indirect draw commands for opaque pass
	m_opaqueIndirectCmdListRef.resize(gfx.GetBackbufferFrameQuantity());
	m_opaqueIndirectCmdListAppend.resize(gfx.GetBackbufferFrameQuantity());

	stl_vector<indirectDrawOpaqueArgs> indirectDrawOpaque;
	indirectDrawOpaque.resize(m_maxNumMeshesToRender);
	for (u32 i = 0; i < m_maxNumMeshesToRender; ++i)
	{
		indirectDrawOpaque[i].IBV.BufferLocation = dx12Context.ResourceGetGPUAddress(m_indexBuffers[i].GFXResourceIndex);
		indirectDrawOpaque[i].IBV.SizeInBytes = m_indexBuffers[i].SizeInBytes;
		indirectDrawOpaque[i].IBV.Format = (u32)cfc::gpu_format_type::R32Uint;

		indirectDrawOpaque[i].VBV.BufferLocation = dx12Context.ResourceGetGPUAddress(m_vertexBuffers[i].GFXResourceIndex);
		indirectDrawOpaque[i].VBV.SizeInBytes = m_vertexBuffers[i].SizeInBytes;
		indirectDrawOpaque[i].VBV.StrideInBytes = m_vertexBuffers[i].StrideInBytes;

		indirectDrawOpaque[i].ModelMatrixIndex = i;
		indirectDrawOpaque[i].AlbedoTextureDescriptorTableIdx = m_materials[m_materialIds[i]].AlbedoGFXResourceDescTableIndex;

		indirectDrawOpaque[i].Draw.IndexCountPerInstance = m_indexBuffers[i].NumIndices;
		indirectDrawOpaque[i].Draw.InstanceCount = 1;
		indirectDrawOpaque[i].Draw.BaseVertexLocation = 0;
		indirectDrawOpaque[i].Draw.StartIndexLocation = 0;
		indirectDrawOpaque[i].Draw.StartInstanceLocation = 0;
	}
	
	m_opaqueIndirectCmdListAppendDescTableOffset.resize(gfx.GetBackbufferFrameQuantity());
	for (u32 i = 0; i < gfx.GetBackbufferFrameQuantity(); ++i)
	{
		m_opaqueIndirectCmdListRef[i] = gfxResourceStream->AddStaticResource(cfc::gfx_resource_type::SRVBuffer, &indirectDrawOpaque[0], sizeof(indirectDrawOpaqueArgs) * indirectDrawOpaque.size());
		
		// note that we add a u32 for the append buffer count
		m_opaqueIndirectCmdListAppend[i].CounterOffsetInBytes = (u32)(sizeof(indirectDrawOpaqueArgs) * indirectDrawOpaque.size());
		m_opaqueIndirectCmdListAppend[i].CounterOffsetInBytes = ((m_opaqueIndirectCmdListAppend[i].CounterOffsetInBytes + 4095) / 4096) * 4096;
		m_opaqueIndirectCmdListAppend[i].AppendBufferGFXResourceIndex = gfxResourceStream->AddDynamicResource(cfc::gfx_resource_type::UAVBuffer, m_opaqueIndirectCmdListAppend[i].CounterOffsetInBytes + sizeof(u32), false);
		gfxResourceStream->UpdateDynamicResource(m_opaqueIndirectCmdListAppend[i].AppendBufferGFXResourceIndex, sizeof(indirectDrawOpaqueArgs) * indirectDrawOpaque.size(), &indirectDrawOpaque[0]);

		m_opaqueIndirectCmdListAppendDescTableOffset[i] = m_albedoTextureGFXResourceIndex.size() + i;

		m_opaqueRenderingDescHeap->SetUAVBuffer(m_opaqueIndirectCmdListAppendDescTableOffset[i], m_opaqueIndirectCmdListAppend[i].AppendBufferGFXResourceIndex, sizeof(indirectDrawOpaqueArgs), 0, m_maxNumMeshesToRender, m_opaqueIndirectCmdListAppend[i].CounterOffsetInBytes, m_opaqueIndirectCmdListAppend[i].AppendBufferGFXResourceIndex);

		sprintf(resourceNameBuffer, "m_opaqueIndirectCmdListRef[%d]", i);
		dx12Context.ResourceSetName(m_opaqueIndirectCmdListRef[i], resourceNameBuffer);

		sprintf(resourceNameBuffer, "m_opaqueIndirectCmdListAppend[%d]", i);
		dx12Context.ResourceSetName(m_opaqueIndirectCmdListAppend[i].AppendBufferGFXResourceIndex, resourceNameBuffer);
	}

	u32 zero[] = { 0,0,0,0 };
	m_opaqueIndirectCmdListAppend[0].AppendBufferCounterResetGfxResourceIndex = gfxResourceStream->AddStaticResource(cfc::gfx_resource_type::CopySource, &zero, sizeof(zero));
	m_opaqueIndirectCmdListAppend[1].AppendBufferCounterResetGfxResourceIndex = m_opaqueIndirectCmdListAppend[0].AppendBufferCounterResetGfxResourceIndex;

	dx12Context.ResourceSetName(m_opaqueIndirectCmdListAppend[0].AppendBufferCounterResetGfxResourceIndex, "m_opaqueIndirectCmdListAppendCounterResetBuffer");

	m_debugRT = gfx.AddRenderTarget2D(gfx.GetBackbufferWidth(), gfx.GetBackbufferHeight(), cfc::gpu_format_type::Rgba8UnormSrgb);
	dx12Context.ResourceSetName(m_debugRT, "m_debugRT");

	gfxResourceStream->Flush();

	u32* tmpDepthBufferFill = new u32[gfx.GetBackbufferWidth() * gfx.GetBackbufferHeight()];

	memset(tmpDepthBufferFill, 0xFF, sizeof(u32) * gfx.GetBackbufferWidth() * gfx.GetBackbufferHeight());
	m_occlusionDepthBufferHalfRes.Width = gfx.GetBackbufferWidth() / HALF_SCREEN_DIV;
	m_occlusionDepthBufferHalfRes.Height = gfx.GetBackbufferHeight() / HALF_SCREEN_DIV;
	m_occlusionDepthBufferHalfRes.UAVRTResource = gfxResourceStream->AddTexture(cfc::gfx_texture_creation_desc(tmpDepthBufferFill, cfc::gpu_format_type::R32Typeless, (i32)m_occlusionDepthBufferHalfRes.Width, (i32)m_occlusionDepthBufferHalfRes.Height, 1, 1, true));
	dx12Context.ResourceSetName(m_occlusionDepthBufferHalfRes.UAVRTResource, "m_occlusionDepthBufferHalfRes");

	gfxResourceStream->Flush();

	m_occlusionDepthBufferQuarterRes.Width = gfx.GetBackbufferWidth() / QUART_SCREEN_DIV;
	m_occlusionDepthBufferQuarterRes.Height = gfx.GetBackbufferHeight() / QUART_SCREEN_DIV;
	m_occlusionDepthBufferQuarterRes.UAVRTResource = gfxResourceStream->AddTexture(cfc::gfx_texture_creation_desc(tmpDepthBufferFill, cfc::gpu_format_type::R32Typeless, (i32)m_occlusionDepthBufferQuarterRes.Width, (i32)m_occlusionDepthBufferQuarterRes.Height, 1, 1, true));
	dx12Context.ResourceSetName(m_occlusionDepthBufferQuarterRes.UAVRTResource, "m_occlusionDepthBufferQuarterRes");

	delete[] tmpDepthBufferFill;

	m_depthBufferDescHeap->SetSRVTexture(0, gfx.GetBackbufferDSResource(), cfc::gpu_format_type::R24UnormX8Typeless);
	m_depthBufferDescHeap->SetUAVTexture(1, m_occlusionDepthBufferHalfRes.UAVRTResource, cfc::gpu_format_type::R32Uint);
	m_depthBufferDescHeap->SetSRVTexture(2, m_occlusionDepthBufferHalfRes.UAVRTResource, cfc::gpu_format_type::R32Float);
	m_depthBufferDescHeap->SetUAVTexture(3, m_occlusionDepthBufferQuarterRes.UAVRTResource, cfc::gpu_format_type::R32Float);
	m_depthBufferDescHeap->SetSRVTexture(4, m_occlusionDepthBufferQuarterRes.UAVRTResource, cfc::gpu_format_type::R32Float);
	m_depthBufferDescHeap->SetSRVTexture(5, gfx.GetRenderTargetResource(m_debugRT), cfc::gpu_format_type::Rgba8UnormSrgb);

	gfxResourceStream->Flush();

	gfxResourceStream->WaitForFinish();

	gfx.RemoveResourceStream(gfxResourceStream->GetIndex());

	m_status = stl_string_advanced::sprintf("Finished loading.");
}

void scene::Unload(cfc::gfx& gfx)
{
	for (u32 i = 0; i < m_vertexBuffers.size(); ++i)
		gfx.RemoveResource(m_vertexBuffers[i].GFXResourceIndex);
	gfx.RemoveResource(m_aabbVertexBuffer.GFXResourceIndex);
	m_vertexBuffers.resize(0);

	for (u32 i = 0; i < m_indexBuffers.size(); ++i)
		gfx.RemoveResource(m_indexBuffers[i].GFXResourceIndex);
	gfx.RemoveResource(m_aabbIndexBuffer.GFXResourceIndex);
	m_indexBuffers.resize(0);

	gfx.RemoveResource(m_modelMatricesGFXResourceIndex);
	m_modelMatrices.resize(0);

	m_materialIds.resize(0);
	for (u32 i = 0; i < m_albedoTextureGFXResourceIndex.size(); ++i)
		gfx.RemoveResource(m_albedoTextureGFXResourceIndex[i]);
	m_albedoTextureGFXResourceIndex.resize(0);

	m_materials.resize(0);

	gfx.RemoveResource(m_aabbTransScaleMatricesGFXResourceIndex);

	for (u32 i = 0; i < m_visibilityBufferGFXResourceIndex.size(); ++i)
		gfx.RemoveResource(m_visibilityBufferGFXResourceIndex[i]);
	m_visibilityBufferGFXResourceIndex.resize(0);

	for (u32 i = 0; i < m_opaqueIndirectCmdListRef.size(); ++i)
		gfx.RemoveResource(m_opaqueIndirectCmdListRef[i]);
	m_opaqueIndirectCmdListRef.resize(0);

	for (u32 i = 0; i < m_opaqueIndirectCmdListAppend.size(); ++i)
		gfx.RemoveResource(m_opaqueIndirectCmdListAppend[i].AppendBufferGFXResourceIndex);
	m_opaqueIndirectCmdListAppend.resize(0);

	gfx.RemoveResource(m_opaqueIndirectCmdListAppend[0].AppendBufferCounterResetGfxResourceIndex);

	gfx.RemoveResource(m_occlusionDepthBufferHalfRes.UAVRTResource);
	gfx.RemoveResource(m_occlusionDepthBufferQuarterRes.UAVRTResource);

	gfx.RemoveRenderTarget(m_debugRT);

	m_aabbs.resize(0);
	m_aabbTransScaleMatrices.resize(0);

	m_downSampleReprojectedDepthBufferCmp.Unload(gfx);
	m_reprojectDepthBufferCmp.Unload(gfx);
	m_copyReprojectDepthBuffer.Unload(gfx);
	m_renderVisibilityGfx.Unload(gfx);
	m_collectVisibleDrawCallsCmp.Unload(gfx);
	m_collectVisibleDrawCallsCmpLowOverhead.Unload(gfx);
	m_renderOpaqueGfx.Unload(gfx);
	m_debugFullScreenTexQuad.Unload(gfx);
	m_debugOpaqueWireFrameGfx.Unload(gfx);

	// timer query groups indirect
	m_timerQueryIndirectDrawFrame.resize(0);
	m_timerQueryClearUav.resize(0);
	m_timerQueryReprojectDepth.resize(0);
	m_timerQueryDownSampleReprojectedDepth.resize(0);
	m_timerQueryCopyUAVToDepth.resize(0);
	m_timerQueryDrawAABBs.resize(0);
	m_timerQueryClearAppendBufferPass.resize(0);
	m_timerQueryAquireVisibleObjects.resize(0);
	m_timerQueryIndirectDraw.resize(0);

	// timer query groups direct
	m_timerQueryDirectDraw.resize(0);

	delete m_opaqueIndirectCmdList;

	gfx.RemoveDescriptorHeap(m_opaqueRenderingDescHeap->GetIndex());
	gfx.RemoveDescriptorHeap(m_depthBufferDescHeap->GetIndex());
}

void scene::Resize(cfc::gfx& gfx)
{
	// DX12 INTEROP
	cfc::gfx_dx12& dx12Gfx = static_cast<cfc::gfx_dx12&>(gfx);
	cfc::gpu_dx12_context& dx12Context = *reinterpret_cast<cfc::gpu_dx12_context*>(dx12Gfx.DX12_GetContext());

	cfc::gfx_resource_stream* gfxResourceStream = gfx.GetResourceStream(gfx.AddResourceStream());

	m_debugRT = gfx.AddRenderTarget2D(gfx.GetBackbufferWidth(), gfx.GetBackbufferHeight(), cfc::gpu_format_type::Rgba8UnormSrgb);
	dx12Context.ResourceSetName(m_debugRT, "m_debugRT");

	gfxResourceStream->Flush();

	u32* tmpDepthBufferFill = new u32[gfx.GetBackbufferWidth() * gfx.GetBackbufferHeight()];

	memset(tmpDepthBufferFill, 0xFF, sizeof(u32) * gfx.GetBackbufferWidth() * gfx.GetBackbufferHeight());
	m_occlusionDepthBufferHalfRes.Width = gfx.GetBackbufferWidth() / HALF_SCREEN_DIV;
	m_occlusionDepthBufferHalfRes.Height = gfx.GetBackbufferHeight() / HALF_SCREEN_DIV;
	m_occlusionDepthBufferHalfRes.UAVRTResource = gfxResourceStream->AddTexture(cfc::gfx_texture_creation_desc(tmpDepthBufferFill, cfc::gpu_format_type::R32Typeless, (i32)m_occlusionDepthBufferHalfRes.Width, (i32)m_occlusionDepthBufferHalfRes.Height, 1, 1, true));
	dx12Context.ResourceSetName(m_occlusionDepthBufferHalfRes.UAVRTResource, "m_occlusionDepthBufferHalfRes");

	gfxResourceStream->Flush();

	m_occlusionDepthBufferQuarterRes.Width = gfx.GetBackbufferWidth() / QUART_SCREEN_DIV;
	m_occlusionDepthBufferQuarterRes.Height = gfx.GetBackbufferHeight() / QUART_SCREEN_DIV;
	m_occlusionDepthBufferQuarterRes.UAVRTResource = gfxResourceStream->AddTexture(cfc::gfx_texture_creation_desc(tmpDepthBufferFill, cfc::gpu_format_type::R32Typeless, (i32)m_occlusionDepthBufferQuarterRes.Width, (i32)m_occlusionDepthBufferQuarterRes.Height, 1, 1, true));
	dx12Context.ResourceSetName(m_occlusionDepthBufferQuarterRes.UAVRTResource, "m_occlusionDepthBufferQuarterRes");

	delete[] tmpDepthBufferFill;


	m_depthBufferDescHeap->SetSRVTexture(0, gfx.GetBackbufferDSResource(), cfc::gpu_format_type::R24UnormX8Typeless);
	m_depthBufferDescHeap->SetUAVTexture(1, m_occlusionDepthBufferHalfRes.UAVRTResource, cfc::gpu_format_type::R32Uint);
	m_depthBufferDescHeap->SetSRVTexture(2, m_occlusionDepthBufferHalfRes.UAVRTResource, cfc::gpu_format_type::R32Float);
	m_depthBufferDescHeap->SetUAVTexture(3, m_occlusionDepthBufferQuarterRes.UAVRTResource, cfc::gpu_format_type::R32Float);
	m_depthBufferDescHeap->SetSRVTexture(4, m_occlusionDepthBufferQuarterRes.UAVRTResource, cfc::gpu_format_type::R32Float);
	m_depthBufferDescHeap->SetSRVTexture(5, gfx.GetRenderTargetResource(m_debugRT), cfc::gpu_format_type::Rgba8UnormSrgb);

	gfxResourceStream->Flush();

	gfxResourceStream->WaitForFinish();

	gfx.RemoveResourceStream(gfxResourceStream->GetIndex());
}

void scene::GatherFrameTimerQueries(cfc::gfx& gfx, OcclusionTypes occlusionType, stl_vector<cfc::gfx_gpu_timer_query>& timerQueriesOUT) const
{
	const usize queryTimerResolvedFrame = gfx.GetTimerQueryResolvedFrameIndex();

	switch (occlusionType)
	{
		case OcclusionTypes::None:
		{
			timerQueriesOUT.push_back(m_timerQueryDirectDraw[queryTimerResolvedFrame]);
			break;
		}
		case OcclusionTypes::Gpu:
		{
			timerQueriesOUT.push_back(m_timerQueryIndirectDrawFrame[queryTimerResolvedFrame]);
			timerQueriesOUT.push_back(m_timerQueryClearUav[queryTimerResolvedFrame]);
			timerQueriesOUT.push_back(m_timerQueryReprojectDepth[queryTimerResolvedFrame]);
			timerQueriesOUT.push_back(m_timerQueryDownSampleReprojectedDepth[queryTimerResolvedFrame]);
			timerQueriesOUT.push_back(m_timerQueryCopyUAVToDepth[queryTimerResolvedFrame]);
			timerQueriesOUT.push_back(m_timerQueryDrawAABBs[queryTimerResolvedFrame]);
			timerQueriesOUT.push_back(m_timerQueryClearAppendBufferPass[queryTimerResolvedFrame]);
			timerQueriesOUT.push_back(m_timerQueryAquireVisibleObjects[queryTimerResolvedFrame]);
			timerQueriesOUT.push_back(m_timerQueryIndirectDraw[queryTimerResolvedFrame]);
			break;
		}
	}
}

stl_string scene::GetStatus()
{
	stl_string ret;
	m_mtxStatus.lock();
	ret = m_status;
	m_mtxStatus.unlock();
	return ret;
}

void scene::Render(cfc::gfx& gfx, cfc::gfx_command_list& cmdList, OcclusionTypes occlusionType, usize viewStateGfxResourceIndex, usize prevViewStateGfxResourceIndex, view_state& view)
{
	const usize queryTimerResolvedFrame = gfx.GetTimerQueryResolvedFrameIndex();

	switch (occlusionType)
	{
		case OcclusionTypes::None:
		{
			renderNoOcclusion(gfx, cmdList, viewStateGfxResourceIndex, view);
			break;
		}
		case OcclusionTypes::Gpu:
		{
			renderGPUOcclusion(gfx, cmdList, viewStateGfxResourceIndex, prevViewStateGfxResourceIndex);
			break;
		}
	}
}

void scene::renderNoOcclusion(cfc::gfx& gfx, cfc::gfx_command_list& cmdList, usize viewStateGfxResourceIndex, view_state& view)
{
	const usize frameIndex = gfx.GetBackbufferFrameIndex();
	const usize timerQueryWriteIndex = gfx.GetTimerQueryWriteFrameIndex();

	m_timerQueryDirectDraw[timerQueryWriteIndex].Begin(&cmdList, "Direct Draw Frame");

	// clear
	float clrColor[] = { 0.0f, 0.0f, 0.2f, 1.0f };
	cmdList.GFXClearRenderTarget(gfx.GetBackbufferRTVOffset(), clrColor);
	cmdList.GFXClearDepthStencilTarget(gfx.GetBackbufferDSVOffset());

	// set output
	cmdList.GFXSetViewports(cfc::gpu_viewport(0, 0, (f32)gfx.GetBackbufferWidth(), (f32)gfx.GetBackbufferHeight()));
	cmdList.GFXSetScissorRects(cfc::gpu_rectangle(0, 0, gfx.GetBackbufferWidth(), gfx.GetBackbufferHeight()));
	cmdList.GFXSetRenderTargets(gfx.GetBackbufferRTVOffset(), gfx.GetBackbufferDSVOffset());
	
	collision::PrimFrustum frustum;
	frustum.SetMatrix(view.ProjectionMatrix * view.ViewMatrix);

	{
		m_renderOpaqueGfx.Begin(&cmdList);
		{
			cmdList.GFXSetPrimitiveTopology(cfc::gpu_primitive_type::TriangleList);
			cmdList.GFXSetRootParameterCBV(1, viewStateGfxResourceIndex, CB_ALIGNMENT_IN_BYTES * frameIndex);

			cmdList.SetDescriptorHeap(m_opaqueRenderingDescHeap);
			cmdList.GFXSetDescriptorTableCbvSrvUav(3, m_defaultMaterial.AlbedoGFXResourceDescTableIndex);

			cmdList.GFXSetRootParameterSRV(0, m_modelMatricesGFXResourceIndex);

			// do draws
			for (u32 i = 0; i < m_maxNumMeshesToRender; ++i)
			{
				collision::PrimAABB* bbox = ((collision::PrimAABB*)&m_final_aabbs[i]);
				if (frustum.Test(*bbox) == false)
					continue;

				u32 meshIndexOpaqueValue[2] = { i, m_materials[m_materialIds[i]].AlbedoGFXResourceDescTableIndex };
				cmdList.GFXSetRootParameterConstants(2, meshIndexOpaqueValue, 2);
				cmdList.GFXSetIndexBuffer(m_indexBuffers[i].GFXResourceIndex, 0, m_indexBuffers[i].SizeInBytes, cfc::gpu_format_type::R32Uint);
				cmdList.GFXSetVertexBuffer(0, m_vertexBuffers[i].GFXResourceIndex, 0, m_vertexBuffers[i].StrideInBytes, m_vertexBuffers[i].SizeInBytes);
				cmdList.GFXDrawIndexedInstanced(m_indexBuffers[i].NumIndices, 1, 0, 0, 0);
			}
		}
	}
	m_timerQueryDirectDraw[timerQueryWriteIndex].End();
}

void scene::renderGPUOcclusion(cfc::gfx& gfx, cfc::gfx_command_list& cmdList, usize viewStateGfxResourceIndex, usize prevViewStateGfxResourceIndex)
{
	const usize frameIndex = gfx.GetBackbufferFrameIndex();
	const usize timerQueryWriteIndex = gfx.GetTimerQueryWriteFrameIndex();

	m_timerQueryIndirectDrawFrame[timerQueryWriteIndex].Begin(&cmdList, "Indirect Draw Frame");

	// clear
	float clrColor[] = { 0.0f, 0.0f, 0.2f, 1.0f };
	cmdList.GFXClearRenderTarget(gfx.GetBackbufferRTVOffset(), clrColor);

	// the following calls will all use the depth buffer descriptor heap, since we are only interested in creating a depth buffer to cull against
	cmdList.SetDescriptorHeap(m_depthBufferDescHeap);

	// SETUP DEBUG DRAW RT
	if (m_debugRenderMode != DebugRenderMode::NoDebugRender)
	{
		cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(gfx.GetRenderTargetResource(m_debugRT), cfc::gpu_resourcestate::PixelShaderResource, cfc::gpu_resourcestate::RenderTarget));
		const usize debugRTVOffset = gfx.GetRenderTargetRTVOffset(m_debugRT);
		cmdList.GFXSetRenderTargets(&debugRTVOffset, 1, cfc::invalid_index);
	}
	else
	{
		cmdList.GFXSetRenderTargets(nullptr, 0, cfc::invalid_index);
	}

	cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(m_occlusionDepthBufferHalfRes.UAVRTResource, cfc::gpu_resourcestate::PixelShaderResource | cfc::gpu_resourcestate::NonPixelShaderResource, cfc::gpu_resourcestate::UnorderedAccess));

	// CLEAR UAV
	{
		m_timerQueryClearUav[timerQueryWriteIndex].Begin(&cmdList, "Clear UAV");
		{
			// DX12 specific, note that we use the DX12 gpu commands directly
			cfc::gfx_dx12& dx12Gfx = static_cast<cfc::gfx_dx12&>(gfx);
			cfc::gpu_dx12_cmdlist_direct_api& dx12CmdList = *reinterpret_cast<cfc::gpu_dx12_cmdlist_direct_api*>(dx12Gfx.DX12_GetDirectCommandListAPI(cmdList.GetIndex()));

			float zero = 0.0;
			u32 floatZeroAsUint = reinterpret_cast<u32&>(zero);
			u32 values[4] = { floatZeroAsUint, floatZeroAsUint, floatZeroAsUint, floatZeroAsUint };
			dx12CmdList.ClearUnorderedAccessViewUint(m_depthBufferDescHeap->GetUAVGPUDescriptorHandle(1), m_depthBufferDescHeap->GetUAVCPUDescriptorHandle(1), m_occlusionDepthBufferHalfRes.UAVRTResource, values, 0, NULL);
		}

		m_timerQueryClearUav[timerQueryWriteIndex].End();
	}

	//cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::UAV(m_occlusionDepthBufferHalfRes.UAVRTResource));

	cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(gfx.GetBackbufferDSResource(), cfc::gpu_resourcestate::DepthWrite, cfc::gpu_resourcestate::NonPixelShaderResource | cfc::gpu_resourcestate::PixelShaderResource));

	// DEBUG RENDER DEPTH BUFFER
	if (m_debugRenderMode == DebugRenderMode::RenderDebugAll || m_debugRenderMode == DebugRenderMode::RenderDebugPreviousDepth)
	{
		debugRenderTexture(gfx, cmdList, 0, 0);
	}

	// REPROJECT PREV ZBUFFER 1/2 res orig res
	{
		m_timerQueryReprojectDepth[timerQueryWriteIndex].Begin(&cmdList, "Reproject Depth");

		m_reprojectDepthBufferCmp.Begin(&cmdList);
		{
			cmdList.CMPSetRootParameterCBV(0, viewStateGfxResourceIndex, CB_ALIGNMENT_IN_BYTES * frameIndex);
			cmdList.CMPSetRootParameterCBV(1, prevViewStateGfxResourceIndex, CB_ALIGNMENT_IN_BYTES * frameIndex);
			cmdList.CMPSetDescriptorTableCbvSrvUav(2, 0); // sets depth buffer as SRV from descriptor table slot 0 and m_occlusionDepthBufferHalfRes as UAV from slot 1

			const u32 dispatchX = (u32)stl_math_iroundupdiv(m_occlusionDepthBufferHalfRes.Width, m_reprojectDepthBufferCmp.GetNumThreadsX());
			const u32 dispatchY = (u32)stl_math_iroundupdiv(m_occlusionDepthBufferHalfRes.Height, m_reprojectDepthBufferCmp.GetNumThreadsY());
			cmdList.CMPDispatch(dispatchX, dispatchY, 1);
		}
		m_timerQueryReprojectDepth[timerQueryWriteIndex].End();
	}

	cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(m_occlusionDepthBufferHalfRes.UAVRTResource, cfc::gpu_resourcestate::UnorderedAccess, cfc::gpu_resourcestate::PixelShaderResource | cfc::gpu_resourcestate::NonPixelShaderResource));

	// DEBUG RENDER REPROJECTED DEPTH
	if (m_debugRenderMode == DebugRenderMode::RenderDebugAll || m_debugRenderMode == DebugRenderMode::RenderDebugReprojectedDepth)
	{
		debugRenderTexture(gfx, cmdList, 2, 1);
	}

	// DOWN SAMPLE REPROJECTED Z BUFFER 1/4 res orig res
	if (m_enableReprojectedDownSample)
	{
		cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(m_occlusionDepthBufferQuarterRes.UAVRTResource, cfc::gpu_resourcestate::PixelShaderResource | cfc::gpu_resourcestate::NonPixelShaderResource, cfc::gpu_resourcestate::UnorderedAccess));

		{
			m_timerQueryDownSampleReprojectedDepth[timerQueryWriteIndex].Begin(&cmdList, "Down Sample Reprojected Depth");
			m_downSampleReprojectedDepthBufferCmp.Begin(&cmdList);
			{
				float screenRes[2] = { gfx.GetBackbufferWidth(), gfx.GetBackbufferHeight() };
				cmdList.CMPSetRootParameterConstants(0, screenRes, 2, 0);
				cmdList.CMPSetDescriptorTableCbvSrvUav(1, 2); // sets m_occlusionDepthBufferHalfRes as SRV from descriptor table slot 2 and m_occlusionDepthBufferQuarterRes as UAV from slot 3

				const u32 dispatchX = (u32)stl_math_iroundupdiv(m_occlusionDepthBufferQuarterRes.Width, m_reprojectDepthBufferCmp.GetNumThreadsX());
				const u32 dispatchY = (u32)stl_math_iroundupdiv(m_occlusionDepthBufferQuarterRes.Height, m_reprojectDepthBufferCmp.GetNumThreadsY());
				cmdList.CMPDispatch(dispatchX, dispatchY, 1);
			}
			m_timerQueryDownSampleReprojectedDepth[timerQueryWriteIndex].End();
		}

		cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(m_occlusionDepthBufferQuarterRes.UAVRTResource, cfc::gpu_resourcestate::UnorderedAccess, cfc::gpu_resourcestate::PixelShaderResource | cfc::gpu_resourcestate::NonPixelShaderResource));

		// DEBUG RENDER DOWNSAMPLED Z BUFFER
		if (m_debugRenderMode == DebugRenderMode::RenderDebugAll || m_debugRenderMode == DebugRenderMode::RenderDebugReprojectedDownSample)
		{
			debugRenderTexture(gfx, cmdList, 4, 2);
		}
	}

	// SETUP VIEWPORT AND SCISSOR
	if (m_enableReprojectedDownSample)
	{
		// set output, note that we move this to the other side of the UAV, since we dont read or write in this area
		cmdList.GFXSetViewports(cfc::gpu_viewport((f32)0, 0, (f32)m_occlusionDepthBufferQuarterRes.Width, (f32)m_occlusionDepthBufferQuarterRes.Height));
		cmdList.GFXSetScissorRects(cfc::gpu_rectangle(0, 0, (long)m_occlusionDepthBufferQuarterRes.Width, (long)m_occlusionDepthBufferQuarterRes.Height));
	}
	else
	{
		// set output, note that we move this to the other side of the UAV, since we dont read or write in this area
		cmdList.GFXSetViewports(cfc::gpu_viewport(0, 0, (f32)m_occlusionDepthBufferHalfRes.Width, (f32)m_occlusionDepthBufferHalfRes.Height));
		cmdList.GFXSetScissorRects(cfc::gpu_rectangle(0, 0, m_occlusionDepthBufferHalfRes.Width, m_occlusionDepthBufferHalfRes.Height));
	}

	cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(gfx.GetBackbufferDSResource(), cfc::gpu_resourcestate::NonPixelShaderResource | cfc::gpu_resourcestate::PixelShaderResource, cfc::gpu_resourcestate::DepthWrite));

	cmdList.GFXSetRenderTargets(nullptr, 0, gfx.GetBackbufferDSVOffset());

	// COPY UAV INTO DEPTH BUFFER
	{
		m_timerQueryCopyUAVToDepth[timerQueryWriteIndex].Begin(&cmdList, "Copy UAV To Depth Pass");
		m_copyReprojectDepthBuffer.Begin(&cmdList);
		{
			cmdList.GFXSetPrimitiveTopology(cfc::gpu_primitive_type::TriangleStrip);

			if (m_enableReprojectedDownSample)
				cmdList.GFXSetDescriptorTableCbvSrvUav(0, 4); // sets m_occlusionDepthBufferQuarterRes as SRV from descriptor table slot 4
			else
				cmdList.GFXSetDescriptorTableCbvSrvUav(0, 2); // sets m_occlusionDepthBufferHalfRes as SRV from descriptor table slot 2

			cmdList.GFXDrawInstanced(3, 1, 0, 0);
		}
		m_timerQueryCopyUAVToDepth[timerQueryWriteIndex].End();
	}

	cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(m_visibilityBufferGFXResourceIndex[frameIndex], cfc::gpu_resourcestate::PixelShaderResource | cfc::gpu_resourcestate::NonPixelShaderResource, cfc::gpu_resourcestate::UnorderedAccess));

	// DRAW AABBs
	{
		m_timerQueryDrawAABBs[timerQueryWriteIndex].Begin(&cmdList, "Indirect Draw: Draw AABBs Pass");
		m_renderVisibilityGfx.Begin(&cmdList);
		{
			cmdList.GFXSetPrimitiveTopology(cfc::gpu_primitive_type::TriangleList);

			cmdList.GFXSetRootParameterSRV(0, m_aabbTransScaleMatricesGFXResourceIndex);
			cmdList.GFXSetRootParameterSRV(1, m_modelMatricesGFXResourceIndex);
			cmdList.GFXSetRootParameterCBV(3, viewStateGfxResourceIndex, CB_ALIGNMENT_IN_BYTES * frameIndex);
			cmdList.GFXSetRootParameterUAV(2, m_visibilityBufferGFXResourceIndex[frameIndex], 0);

			cmdList.GFXSetIndexBuffer(m_aabbIndexBuffer.GFXResourceIndex, 0, m_aabbIndexBuffer.SizeInBytes, cfc::gpu_format_type::R32Uint);
			cmdList.GFXSetVertexBuffer(0, m_aabbVertexBuffer.GFXResourceIndex, 0, m_aabbVertexBuffer.StrideInBytes, m_aabbVertexBuffer.SizeInBytes);

			// do draws using instancing
			cmdList.GFXDrawIndexedInstanced(m_aabbIndexBuffer.NumIndices, m_maxNumMeshesToRender, 0, 0, 0);
		}
		m_timerQueryDrawAABBs[timerQueryWriteIndex].End();
	}

	cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(m_visibilityBufferGFXResourceIndex[frameIndex], cfc::gpu_resourcestate::UnorderedAccess, cfc::gpu_resourcestate::PixelShaderResource | cfc::gpu_resourcestate::NonPixelShaderResource));
	
	cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(m_opaqueIndirectCmdListAppend[frameIndex].AppendBufferGFXResourceIndex, cfc::gpu_resourcestate::IndirectArgument, cfc::gpu_resourcestate::CopyDestination));

	// CLEAR APPEND BUFFER COUNTER
	{
		m_timerQueryClearAppendBufferPass[timerQueryWriteIndex].Begin(&cmdList, "Indirect Draw: Clear Append Buffer Pass");
		cfc::gfx_dx12& dx12Gfx = static_cast<cfc::gfx_dx12&>(gfx);
		cfc::gpu_dx12_cmdlist_direct_api& dx12CmdList = *reinterpret_cast<cfc::gpu_dx12_cmdlist_direct_api*>(dx12Gfx.DX12_GetDirectCommandListAPI(cmdList.GetIndex()));
		dx12CmdList.CopyBufferRegion(m_opaqueIndirectCmdListAppend[frameIndex].AppendBufferGFXResourceIndex, m_opaqueIndirectCmdListAppend[frameIndex].CounterOffsetInBytes, m_opaqueIndirectCmdListAppend[frameIndex].AppendBufferCounterResetGfxResourceIndex, 0, sizeof(u32));
		m_timerQueryClearAppendBufferPass[timerQueryWriteIndex].End();
	}

	cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(m_opaqueIndirectCmdListAppend[frameIndex].AppendBufferGFXResourceIndex, cfc::gpu_resourcestate::CopyDestination, cfc::gpu_resourcestate::UnorderedAccess));

	// we move the clear of the depth stencil target to after visibility rendering
	cmdList.GFXClearDepthStencilTarget(gfx.GetBackbufferDSVOffset());

	// from here on we are interested in setting the descriptor heap for visually rendering the scene
	cmdList.SetDescriptorHeap(m_opaqueRenderingDescHeap);

	// COLLECT VISIBLE OBJECTS
	{
		m_timerQueryAquireVisibleObjects[timerQueryWriteIndex].Begin(&cmdList, "Indirect Draw: Acquire Visible Objects Pass");
		// according to NVidia paper, launch overhead of compute for simple tasks is not negligible (page 33)
		// http://on-demand.gputechconf.com/gtc/2016/presentation/s6138-christoph-kubisch-pierre-boudier-gpu-driven-rendering.pdf 
		if (!m_enableNvidiaVertexShaderTrick) // use compute
		{
			m_collectVisibleDrawCallsCmp.Begin(&cmdList);
			{
				cmdList.CMPSetRootParameterSRV(0, m_visibilityBufferGFXResourceIndex[frameIndex], 0);
				cmdList.CMPSetRootParameterSRV(1, m_opaqueIndirectCmdListRef[frameIndex]);

				cmdList.CMPSetDescriptorTableCbvSrvUav(3, m_opaqueIndirectCmdListAppendDescTableOffset[frameIndex]);
				cmdList.CMPSetRootParameterConstants(2, &m_maxNumMeshesToRender, 1);

				cmdList.CMPDispatch(stl_math_iroundupdiv(m_maxNumMeshesToRender, m_collectVisibleDrawCallsCmp.GetNumThreadsX()), 1, 1);
			}
		}
		else // use vertex shader as compute
		{
			m_collectVisibleDrawCallsCmpLowOverhead.Begin(&cmdList);
			{
				cmdList.GFXSetRootParameterSRV(0, m_visibilityBufferGFXResourceIndex[frameIndex], 0);
				cmdList.GFXSetRootParameterSRV(1, m_opaqueIndirectCmdListRef[frameIndex]);

				cmdList.SetDescriptorHeap(m_opaqueRenderingDescHeap);
				cmdList.GFXSetDescriptorTableCbvSrvUav(2, m_opaqueIndirectCmdListAppendDescTableOffset[frameIndex]);

				cmdList.GFXSetPrimitiveTopology(cfc::gpu_primitive_type::PointList);
				cmdList.GFXDrawInstanced(m_maxNumMeshesToRender, 1, 0, 0);
			}
		}
		m_timerQueryAquireVisibleObjects[timerQueryWriteIndex].End();
	}

	cmdList.ExecuteBarrier(cfc::gpu_resourcebarrier_desc::Transition(m_opaqueIndirectCmdListAppend[frameIndex].AppendBufferGFXResourceIndex, cfc::gpu_resourcestate::UnorderedAccess, cfc::gpu_resourcestate::IndirectArgument));
	
	cmdList.GFXSetViewports(cfc::gpu_viewport(0, 0, (f32)gfx.GetBackbufferWidth(), (f32)gfx.GetBackbufferHeight()));
	cmdList.GFXSetScissorRects(cfc::gpu_rectangle(0, 0, gfx.GetBackbufferWidth(), gfx.GetBackbufferHeight()));
	cmdList.GFXSetRenderTargets(gfx.GetBackbufferRTVOffset(), gfx.GetBackbufferDSVOffset());

	// DRAW VISIBLE OBJECTS
	{
		m_timerQueryIndirectDraw[timerQueryWriteIndex].Begin(&cmdList, "Indirect Draw: Draw Visible Objects Pass");

		m_renderOpaqueGfx.Begin(&cmdList);
		{
			cmdList.GFXSetPrimitiveTopology(cfc::gpu_primitive_type::TriangleList);
			cmdList.GFXSetRootParameterCBV(1, viewStateGfxResourceIndex, CB_ALIGNMENT_IN_BYTES * frameIndex);
			cmdList.GFXSetRootParameterSRV(0, m_modelMatricesGFXResourceIndex);
			cmdList.GFXSetDescriptorTableCbvSrvUav(3, m_defaultMaterial.AlbedoGFXResourceDescTableIndex);

			// DX12 specific, note that we use the DX12 gpu commands directly
			cfc::gfx_dx12& dx12Gfx = static_cast<cfc::gfx_dx12&>(gfx);
			cfc::gpu_dx12_cmdlist_direct_api& dx12CmdList = *reinterpret_cast<cfc::gpu_dx12_cmdlist_direct_api*>(dx12Gfx.DX12_GetDirectCommandListAPI(cmdList.GetIndex()));
			dx12CmdList.ExecuteIndirect(m_opaqueIndirectCmdList, m_maxNumMeshesToRender, m_opaqueIndirectCmdListAppend[frameIndex].AppendBufferGFXResourceIndex, 0, m_opaqueIndirectCmdListAppend[frameIndex].AppendBufferGFXResourceIndex, m_opaqueIndirectCmdListAppend[frameIndex].CounterOffsetInBytes);
		}
		m_timerQueryIndirectDraw[timerQueryWriteIndex].End();

		// DRAW VISIBLE OBJECTX WIRE FRAME ON TOP
		if (m_rasterizeWireFrameOfVisibleGeometryAdditive)
		{
			m_debugOpaqueWireFrameGfx.Begin(&cmdList);
			{
				cmdList.GFXSetPrimitiveTopology(cfc::gpu_primitive_type::TriangleList);
				cmdList.GFXSetRootParameterCBV(1, viewStateGfxResourceIndex, CB_ALIGNMENT_IN_BYTES * frameIndex);
				cmdList.GFXSetRootParameterSRV(0, m_modelMatricesGFXResourceIndex);
				cmdList.GFXSetDescriptorTableCbvSrvUav(3, m_defaultMaterial.AlbedoGFXResourceDescTableIndex);

				cfc::gfx_dx12& dx12Gfx = static_cast<cfc::gfx_dx12&>(gfx);
				cfc::gpu_dx12_cmdlist_direct_api& dx12CmdList = *reinterpret_cast<cfc::gpu_dx12_cmdlist_direct_api*>(dx12Gfx.DX12_GetDirectCommandListAPI(cmdList.GetIndex()));
				dx12CmdList.ExecuteIndirect(m_opaqueIndirectCmdList, m_maxNumMeshesToRender, m_opaqueIndirectCmdListAppend[frameIndex].AppendBufferGFXResourceIndex, 0, m_opaqueIndirectCmdListAppend[frameIndex].AppendBufferGFXResourceIndex, m_opaqueIndirectCmdListAppend[frameIndex].CounterOffsetInBytes);
			}
		}
	}

	m_timerQueryIndirectDrawFrame[timerQueryWriteIndex].End();

	// DRAW DEBUG RT
	if (m_debugRenderMode != DebugRenderMode::NoDebugRender)
	{
		if(m_debugRenderMode == RenderDebugAll)
			cmdList.GFXSetScissorRects(cfc::gpu_rectangle(0, 0, gfx.GetBackbufferWidth(), gfx.GetBackbufferHeight()/3));
		
		cmdList.GFXSetRenderTargets(gfx.GetBackbufferRTVOffset(), cfc::invalid_index);
		cmdList.SetDescriptorHeap(m_depthBufferDescHeap);
		m_debugFullScreenTexQuad.Begin(&cmdList);
		{
			cmdList.GFXSetPrimitiveTopology(cfc::gpu_primitive_type::TriangleList);
			cmdList.GFXSetDescriptorTableCbvSrvUav(0, 5);
			cmdList.GFXDrawInstanced(3, 1, 0, 0);
		}
		cmdList.GFXSetRenderTargets(gfx.GetBackbufferRTVOffset(), gfx.GetBackbufferDSVOffset());
		cmdList.GFXSetScissorRects(cfc::gpu_rectangle(0, 0, gfx.GetBackbufferWidth(), gfx.GetBackbufferHeight()));
	}
}


void scene::debugRenderTexture(cfc::gfx& gfx, cfc::gfx_command_list& cmdList, usize descriptorTableSrvIndex, u32 debugIndex)
{
	u32 widthPerFrame = gfx.GetBackbufferWidth() / 3;
	u32 heightPerFrame = gfx.GetBackbufferHeight() / 3;

	u32 xOffset = widthPerFrame * debugIndex;
	u32 yOffset = 0;

	// DEBUG DRAW CLEARED UAV
	{
		if (m_debugRenderMode == DebugRenderMode::RenderDebugAll)
		{
			cmdList.GFXSetViewports(cfc::gpu_viewport((f32)xOffset, (f32)yOffset, (f32)widthPerFrame, (f32)heightPerFrame));
			cmdList.GFXSetScissorRects(cfc::gpu_rectangle(xOffset, yOffset, xOffset + widthPerFrame, yOffset + heightPerFrame));
		}
		else
		{
			cmdList.GFXSetViewports(cfc::gpu_viewport((f32)0, 0, (f32)gfx.GetBackbufferWidth(), (f32)gfx.GetBackbufferHeight()));
			cmdList.GFXSetScissorRects(cfc::gpu_rectangle(0, 0, gfx.GetBackbufferWidth(), gfx.GetBackbufferHeight()));
		}

		m_debugFullScreenTexQuad.Begin(&cmdList);
		{
			cmdList.GFXSetPrimitiveTopology(cfc::gpu_primitive_type::TriangleList);
			cmdList.GFXSetDescriptorTableCbvSrvUav(0, descriptorTableSrvIndex); // sets m_occlusionDepthBufferHalfRes as SRV from descriptor table slot 2
			cmdList.GFXDrawInstanced(3, 1, 0, 0);
		}

		if (m_debugRenderMode == DebugRenderMode::RenderDebugAll)
		{

			cmdList.GFXSetViewports(cfc::gpu_viewport((f32)0, 0, (f32)gfx.GetBackbufferWidth(), (f32)gfx.GetBackbufferHeight()));
			cmdList.GFXSetScissorRects(cfc::gpu_rectangle(0, 0, gfx.GetBackbufferWidth(), gfx.GetBackbufferHeight()));
		}
	}
}

void scene::setStatus(const stl_string& str)
{
	m_mtxStatus.lock();
	m_status = str;
	m_mtxStatus.unlock();
}
