#pragma once

#include <cfc/base.h>
#include <cfc/core/context.h>
#include <cfc/stl/stl_pimpl.hpp>
#include <cfc/stl/stl_string.hpp>
#include <cfc/stl/stl_vector.hpp>

#define CFC_MAX_GPU_INPUT_LAYOUTS 16

namespace cfc {

	#define CFC_GPU_NODEMASK0()                                     (0u) // special mask: all
	#define CFC_GPU_NODEMASK1(node0)                                ((1u << node0))
	#define CFC_GPU_NODEMASK2(node0, node1)                         ((1u << node0) | (1u << node1))
	#define CFC_GPU_NODEMASK3(node0, node1, node2)                  ((1u << node0) | (1u << node1) | (1u << node2))
	#define CFC_GPU_NODEMASK4(node0, node1, node2, node3)           ((1u << node0) | (1u << node1) | (1u << node2) | (1u << node3))

	enum class gpu_object_type
	{
		CommandQueue,
		CommandAllocator,
		CommandList,
		Fence,
		FenceEvent,
		DescriptorHeap,
		QueryHeap,
		SwapChain,
		ShaderBlob,
		RootSignature,
		PipelineState,
		Resource,
	};

	enum class gpu_commandlist_type
	{
		Direct,									// command list is used directly to draw to issue commands to GPU (supports Graphics, Compute & Copy)
		Bundle,									// command list is a subset (bundle) of a direct commandlist (not supported for Compute or Copy)
		Compute,								// command list is specifically for compute commands
		Copy									// command list is specifically for copy commands
	};

	enum class gpu_descriptorheap_type
	{
		CbvSrvUav,								// descriptor heap that can contain constant buffer views, shader resource views & unordered access views (can be gpu visible)
		Samplers,								// descriptor heap that can contain sampler states (can be gpu visible)
		Rtv,									// descriptor heap that can contain render target views (only cpu visible)
		Dsv,									// descriptor heap that can contain depth stencil views (only cpu visible)
	};

	enum class gpu_queryheap_type
	{
		Occlusion,								// query heap that can contain occlusion query results
		TimeStamp,								// query heap that can contain gpu timestamp results
	};

	enum class gpu_fenceshare_type
	{
		Unshared,								// fence is only valid for current node & is unshared
		Shared,									// fence is only valid for current node & is shared
		SharedAcrossNodes,						// fence is shared and can be used across devices (nodes)
	};

	enum class gpu_swapflip_type
	{
		Discard,								// discard previous buffer when buffer is flipped	
		Sequential,								// retain previous buffer when buffer is flipped
	};

	enum class gpu_swapimage_type
	{
		Rgba8Unorm,							// R8G8B8A8 Unsigned Normalized
		Rgba8UnormSrgb,						// R8G8B8A8 Unsigned Normalized (contains linear color values - sRGB)
		Rgb10A2Unorm,						// R10G10B10A2 Unsigned Normalized
		Rgba16FloatSrgb,						// R16G16B16A16 Floating Point (contains linear color values - sRGB)
	};

	enum class gpu_heap_type
	{
		Default = 1,							// GPU heap
		Upload,									// CPU staging heap (for uploading from GPU)
		Readback,								// CPU staging heap (for downloading from GPU)
		Custom,									// custom heap (?)
	};

	enum class gpu_primitive_type
	{
		Undefined = 0,
		PointList = 1,
		LineList = 2,
		LineStrip = 3,
		TriangleList = 4,
		TriangleStrip = 5,
		AdjLineList = 10,
		AdjLineStrip = 11,
		AdjTriangleList = 12,
		AdjTriangleStrip = 13,
	};

	enum class gpu_primitivetopology_type
	{
		Undefined = 0,
		Point = 1,
		Line = 2,
		Triangle = 3,
		Patch = 4,
	};

	enum class gpu_format_type
	{
		Unknown = 0,
		Rgba32Typeless = 1,
		Rgba32Float = 2,
		Rgba32Uint = 3,
		Rgba32Sint = 4,
		Rgb32Typeless = 5,
		Rgb32Float = 6,
		Rgb32Uint = 7,
		Rgb32Sint = 8,
		Rgba16Typeless = 9,
		Rgba16Float = 10,
		Rgba16Unorm = 11,
		Rgba16Uint = 12,
		Rgba16Snorm = 13,
		Rgba16Sint = 14,
		Rg32Typeless = 15,
		Rg32Float = 16,
		Rg32Uint = 17,
		Rg32Sint = 18,
		R32G8X24Typeless = 19,
		D32FloatS8X24Uint = 20,
		R32FloatX8X24Typeless = 21,
		X32TypelessG8X24Uint = 22,
		Rgb10A2Typeless = 23,
		Rgb10A2Unorm = 24,
		Rgb10A2Uint = 25,
		Rg11B10Float = 26,
		Rgba8Typeless = 27,
		Rgba8Unorm = 28,
		Rgba8UnormSrgb = 29,
		Rgba8Uint = 30,
		Rgba8Snorm = 31,
		Rgba8Sint = 32,
		Rg16Typeless = 33,
		Rg16Float = 34,
		Rg16Unorm = 35,
		Rg16Uint = 36,
		Rg16Snorm = 37,
		Rg16Sint = 38,
		R32Typeless = 39,
		D32Float = 40,
		R32Float = 41,
		R32Uint = 42,
		R32Sint = 43,
		R24G8Typeless = 44,
		D24UnormS8Uint = 45,
		R24UnormX8Typeless = 46,
		X24TypelessG8Uint = 47,
		Rg8Typeless = 48,
		Rg8Unorm = 49,
		Rg8Uint = 50,
		Rg8Snorm = 51,
		Rg8Sint = 52,
		R16Typeless = 53,
		R16Float = 54,
		D16Unorm = 55,
		R16Unorm = 56,
		R16Uint = 57,
		R16Snorm = 58,
		R16Sint = 59,
		R8Typeless = 60,
		R8Unorm = 61,
		R8Uint = 62,
		R8Snorm = 63,
		R8Sint = 64,
		A8Unorm = 65,
		R1Unorm = 66,
		Rgb9E5Sharedexp = 67,
		Rg8_Bg8Unorm = 68,
		Gr8_Gb8Unorm = 69,
		BC1Typeless = 70,
		BC1Unorm = 71,
		BC1UnormSrgb = 72,
		BC2Typeless = 73,
		BC2Unorm = 74,
		BC2UnormSrgb = 75,
		BC3Typeless = 76,
		BC3Unorm = 77,
		BC3UnormSrgb = 78,
		BC4Typeless = 79,
		BC4Unorm = 80,
		BC4Snorm = 81,
		BC5Typeless = 82,
		BC5Unorm = 83,
		BC5Snorm = 84,
		B5G6R5Unorm = 85,
		B5G5R5A1Unorm = 86,
		Bgra8Unorm = 87,
		Bgrx8Unorm = 88,
		Rgb10XrBiasA2Unorm = 89,
		Bgra8Typeless = 90,
		Bgra8UnormSrgb = 91,
		Bgrx8Typeless = 92,
		Bgrx8UnormSrgb = 93,
		BC6HTypeless = 94,
		BC6H_UF16 = 95,
		BC6H_SF16 = 96,
		BC7Typeless = 97,
		BC7Unorm = 98,
		BC7UnormSrgb = 99,
		AYUV = 100,
		Y410 = 101,
		Y416 = 102,
		NV12 = 103,
		P010 = 104,
		P016 = 105,
		T420Opaque = 106,
		YUY2 = 107,
		Y210 = 108,
		Y216 = 109,
		NV11 = 110,
		AI44 = 111,
		IA44 = 112,
		P8 = 113,
		A8P8 = 114,
		B4G4R4A4Unorm = 115,

		P208 = 130,
		V208 = 131,
		V408 = 132,
		Count
	};

	class CFC_API gpu_format_type_query
	{
	public:
		static bool IsDepthType(gpu_format_type fmt);
		static bool IsStencilType(gpu_format_type fmt);
		static bool IsCompressedType(gpu_format_type fmt);
		static i32 GetBitsPerChannel(gpu_format_type fmt);
		static i32 GetNumChannels(gpu_format_type fmt);
		static i32 GetBitsPerPixel(gpu_format_type fmt);
		static i32 GetCompressedBlockSize(gpu_format_type fmt);
		static usize GetRowBytes(gpu_format_type fmt, usize width);
	};

	enum class gpu_query_type
	{
		Occlusion = 0,
		BinaryOcclusion = 1,
		Timestamp = 2,
		PipelineStatistics = 3,
		StreamOutStatisticsStream0 = 4,
		StreamOutStatisticsStream1 = 5,
		StreamOutStatisticsStream2 = 6,
		StreamOutStatisticsStream3 = 7
	};

	enum class gpu_resourcebarrier_type
	{
		Transition = 0,				// resource transition from one access pattern to a different one
		Aliasing,					// resource is used as another resource (aliased)
		Uav							// resource has to do a uav synchronization
	};

	enum class gpu_resourcebarrierflag_type
	{
		None = 0,					// do a full barrier transition
		BeginOnly,					// starts a barrier transition in a new state, putting a resource in a temporary no-access condition. (used in Synchronization and Multi-Engine)
		EndOnly,					// completes a transition, setting a new state and restoring active access to a resource. (used in Synchronization and Multi-Engine)
	};

	struct gpu_resourcestate
	{
	public:
		// note: [read & write] are from the perspective of the GPU (device)
		//		 ([read]: device will be able to read from this object,
		//	      [write]: device will be able to write to this object)

		enum
		{
			Common = 0,							// resource has no transition flags
			Present = 0,						// resource has no transition flags (ready to present)
			VertexAndConstantBuffer = 0x1,		// resource is accessible as vertex and constant buffer [read] (VB/CBV)
			IndexBuffer = 0x2,					// resource is accessible as index buffer [read] (IB)
			RenderTarget = 0x4,					// resource is accessible as render target [write] (RTV)
			UnorderedAccess = 0x8,				// resource is accessible as unordered access [read/write] (UAV)
			DepthWrite = 0x10,					// resource can be written to as depth target [write] (DTV)
			DepthRead = 0x20,					// resource can be read from as depth source [read] (DSV)
			NonPixelShaderResource = 0x40,		// resource can be used as shader resource in a shader other than pixel shader [read] (SRV)
			PixelShaderResource = 0x80,			// resource can be used as shader resource in a pixel shader [read] (SRV)
			StreamOut = 0x100,					// resource is a stream out target [write]
			IndirectArgument = 0x200,			// resource contains indirect arguments [read]
			Predication = 0x200,				// resource contains predication results [read]
			CopyDestination = 0x400,			// resource is a target for a copy [write]
			CopySource = 0x800,					// resource is a source for a copy [read]
			ResolveDestination = 0x1000,
			ResolveSource = 0x2000,

			// resource can be read by GPU in almost any way (except as a depth source, due to depth compression) [read]
			// note: GenericRead can be inefficient for some devices - try to specialize where possible.
			GenericRead = (((((VertexAndConstantBuffer | IndexBuffer) | NonPixelShaderResource) | PixelShaderResource) | IndirectArgument) | CopySource),
		};

		typedef u32 flag;
	};

	enum class gpu_samplerfilter_type
	{
		MinMagMipPoint = 0,
		MinMagPointMipLinear = 0x1,
		MinPointMagLinearMipPoint = 0x4,
		MinPointMagMipLinear = 0x5,
		MinLinearMagMipPoint = 0x10,
		MinLinearMagPointMipLinear = 0x11,
		MinMagLinearMipPoint = 0x14,
		MinMagMipLinear = 0x15,
		Anisotropic = 0x55,
		CmpMinMagMipPoint = 0x80 + 0,
		CmpMinMagPointMipLinear = 0x80 + 0x1,
		CmpMinPointMagLinearMipPoint = 0x80 + 0x4,
		CmpMinPointMagMipLinear = 0x80 + 0x5,
		CmpMinLinearMagMipPoint = 0x80 + 0x10,
		CmpMinLinearMagPointMipLinear = 0x80 + 0x11,
		CmpMinMagLinearMipPoint = 0x80 + 0x14,
		CmpMinMagMipLinear = 0x80 + 0x15,
		CmpAnisotropic = 0x80 + 0x55,
		MinimumMinMagMipPoint = 0x100 + 0,
		MinimumMinMagPointMipLinear = 0x100 + 0x1,
		MinimumMinPointMagLinearMipPoint = 0x100 + 0x4,
		MinimumMinPointMagMipLinear = 0x100 + 0x5,
		MinimumMinLinearMagMipPoint = 0x100 + 0x10,
		MinimumMinLinearMagPointMipLinear = 0x100 + 0x11,
		MinimumMinMagLinearMipPoint = 0x100 + 0x14,
		MinimumMinMagMipLinear = 0x100 + 0x15,
		MinimumAnisotropic = 0x100 + 0x55,
		MaximumMinMagMipPoint = 0x180 + 0,
		MaximumMinMagPointMipLinear = 0x180 + 0x1,
		MaximumMinPointMagLinearMipPoint = 0x180 + 0x4,
		MaximumMinPointMagMipLinear = 0x180 + 0x5,
		MaximumMinLinearMagMipPoint = 0x180 + 0x10,
		MaximumMinLinearMagPointMipLinear = 0x180 + 0x11,
		MaximumMinMagLinearMipPoint = 0x180 + 0x14,
		MaximumMinMagMipLinear = 0x180 + 0x15,
		MaximumAnisotropic = 0x180 + 0x55,
	};

	enum class gpu_texaddr_type
	{
		Wrap = 1,
		Mirror = 2,
		Clamp = 3,
		Border = 4,
		MirrorOnce = 5
	};

	enum class gpu_shadervisibility_type
	{
		All = 0,
		Compute = 0,
		Vertex = 1,
		Hull = 2,
		Domain = 3,
		Geometry = 4,
		Pixel = 5,
	};

	enum class gpu_comparisonfunc_type
	{
		Never = 1,
		Less = 2,
		Equal = 3,
		LessEqual = 4,
		Greater = 5,
		NotEqual = 6,
		GreaterEqual = 7,
		Always = 8
	};

	enum class gpu_staticbordercolor_type
	{
		TransparentBlack = 0,
		OpaqueBlack = 1,
		OpaqueWhite = 2
	};

	enum class gpu_blendmode_type
	{
		Zero = 1,
		One = 2,
		SrcColor = 3,
		InvSrcColor = 4,
		SrcAlpha = 5,
		InvSrcAlpha = 6,
		DestAlpha = 7,
		InvDestAlpha = 8,
		DestColor = 9,
		InvDestColor = 10,
		SrcAlphaSat = 11,
		BlendFactor = 14,
		InvBlendFactor = 15,
		Src1Color = 16,
		InvSrc1Color = 17,
		Src1Alpha = 18,
		InvSrc1Alpha = 19
	};

	enum class gpu_blendop_type
	{
		Add = 1,
		Subtract = 2,
		ReverseSubtract = 3,
		Min = 4,
		Max = 5
	};

	enum class gpu_logicop_type
	{
		Clear = 0,
		Set = 1,
		Copy = 2,
		CopyInverted = 3,
		NoOp = 4,
		Invert = 5,
		And = 6,
		Nand = 7,
		Or = 8,
		Nor = 9,
		Xor = 10,
		Equiv = 11,
		AndReverse = 12,
		AndInverted = 13,
		OrReverse = 14,
		OrInverted = 15,
	};

	enum class gpu_depthwritemask_type
	{
		Zero = 0,
		All = 1
	};

	enum class gpu_stencilop_type
	{
		Keep = 1,
		Zero = 2,
		Replace = 3,
		IncrSat = 4,
		DecrSat = 5,
		Invert = 6,
		Increment = 7,
		Decrement = 8
	};

	enum class gpu_tristripcutvalue_type
	{
		IbscvDisabled = 0,
		Ibscv0xFFFF = 1,
		Ibscv0xFFFFFFFF = 2
	};

	enum class gpu_pageproperty_type
	{
		Unknown = 0,
		NotAvailable = 1,
		WriteCombine = 2,
		WriteBack = 3
	};

	struct gpu_graphicspipelinestate_desc
	{
		gpu_graphicspipelinestate_desc() {}
		gpu_graphicspipelinestate_desc(usize rootSignatureIdx, usize vertexShaderIdx, usize pixelShaderIdx, gpu_primitivetopology_type primitiveTopology = gpu_primitivetopology_type::Triangle, gpu_format_type rtvFormat = gpu_format_type::Unknown, gpu_format_type dsvFormat = gpu_format_type::Unknown) : RootSignatureIdx(rootSignatureIdx), VertexShaderIdx(vertexShaderIdx), PixelShaderIdx(pixelShaderIdx), PrimitiveTopology(primitiveTopology), NumRenderTargets(rtvFormat == gpu_format_type::Unknown ? 0 : 1), DSVFormat(dsvFormat) { RTVFormats[0] = rtvFormat; if (DSVFormat != gpu_format_type::Unknown) DepthStencilState.DepthEnable = true; }

		// root signature is mandatory - creation will fail with an invalid index.
		usize RootSignatureIdx = cfc::invalid_index;

		// shaders are optional.
		usize VertexShaderIdx = cfc::invalid_index;
		usize HullShaderIdx = cfc::invalid_index;
		usize DomainShaderIdx = cfc::invalid_index;
		usize GeometryShaderIdx = cfc::invalid_index;
		usize PixelShaderIdx = cfc::invalid_index;

		// default assumes triangle topology
		gpu_primitivetopology_type PrimitiveTopology = gpu_primitivetopology_type::Triangle;

		// default assumes no depth buffer & no render buffers
		u32 NumRenderTargets = 0;
		gpu_format_type RTVFormats[8] = { gpu_format_type::Unknown };
		gpu_format_type DSVFormat = gpu_format_type::Unknown;

		struct inputelement_desc
		{
			enum cls_type
			{
				IscPerVertexData = 0,
				IscPerInstanceData = 1
			};
			stl_string SemanticName;			//!< name of semantic - e.g. "POSITION"
			u32 SemanticIndex = 0;						//!< index of semantic - usually set to 0 (first occurrence of POSITION)
			gpu_format_type Format = gpu_format_type::Unknown;	//!< data format (usually R32G32B32A32_FLOAT or some kind of variant)
			u32 InputSlot = 0;							//!< vertex array slot binding number (used for multiple bound vertex buffer inputs)
			u32 AlignedByteOffset = 0;					//!< offset in bytes between elements
			cls_type InputSlotClass = IscPerVertexData;	//!< type of data (per vertex or per instance)
			u32 InstanceDataStepRate = 0;				//!< step rate for instance data, 0 when instancing is not used.

			inline void Set(const i8* semanticName, gpu_format_type fmt, i32 alignedByteOffset, i32 semIdx = 0) { SemanticName = semanticName; Format = fmt; AlignedByteOffset = alignedByteOffset; SemanticIndex = semIdx; }
		};

		// default input elements are empty
		inputelement_desc InputElements[CFC_MAX_GPU_INPUT_LAYOUTS];
		usize NumInputElements() const { for (usize i = 0; i < CFC_MAX_GPU_INPUT_LAYOUTS; i++) { if (InputElements[i].SemanticName.empty()) return i; } return CFC_MAX_GPU_INPUT_LAYOUTS; }

		// TODO: Stream output
		//streamout_desc StreamOutState;

		// default is solid fill with backface culling, where front faced polygon is clockwise, and no conservative rasterization/line antialiasing/depth bias
		struct rasterizer_desc
		{
			enum fillmode_type
			{
				FillWireframe = 2,
				FillSolid = 3
			};

			enum cullmode_type
			{
				CullNone = 1,
				CullFront = 2,
				CullBack = 3
			};

			enum conservativerasterizationmode_type
			{
				CrmOff = 0,
				CrmOn = 1
			};

			fillmode_type FillMode = FillSolid;
			cullmode_type CullMode = CullBack;
			i32 FrontCounterClockwise = 0;
			i32 DepthBias = 0;
			f32 DepthBiasClamp = 0.0f;
			f32 SlopeScaledDepthBias = 0.0f;
			i32 DepthClipEnable = 1;
			i32 MultisampleEnable = 0;
			i32 AntialiasedLineEnable = 0;
			u32 ForcedSampleCount = 0;
			conservativerasterizationmode_type ConservativeRaster = CrmOff;
		} RasterizerState;

		// default blend state is a disabled replace blend, writing to every color channel
		struct rtblendstate_desc
		{
			bool BlendEnabled = false;
			bool LogicOpEnabled = false;
			gpu_blendmode_type SrcBlend = gpu_blendmode_type::One;
			gpu_blendmode_type DstBlend = gpu_blendmode_type::Zero;
			gpu_blendop_type BlendOp = gpu_blendop_type::Add;

			gpu_blendmode_type SrcBlendAlpha = gpu_blendmode_type::One;
			gpu_blendmode_type DstBlendAlpha = gpu_blendmode_type::Zero;
			gpu_blendop_type BlendOpAlpha = gpu_blendop_type::Add;
			gpu_logicop_type LogicOp = gpu_logicop_type::NoOp;
			u8 RenderTargetWriteMask = (1 | 2 | 4 | 8); // bit 1: red, bit 2: green, bit 3 (4): blue, bit 4 (8): alpha
		};

		struct blendstate_desc
		{
			bool AlphaToCoverageEnable = false;
			bool IndependentBlendEnable = false;
			rtblendstate_desc RenderTarget[8];
		} BlendState;

		// default depth stencil is depth read disabled, depth write enabled, comparison:less than, and no stenciling
		struct depthstencil_desc
		{
			bool DepthEnable = false;
			gpu_depthwritemask_type DepthWriteMask = gpu_depthwritemask_type::All;
			gpu_comparisonfunc_type DepthFunc = gpu_comparisonfunc_type::Less;
			bool StencilEnable = false;
			u8 StencilReadMask = 0U;
			u8 StencilWriteMask = 0U;

			struct depthstencilop_desc
			{
				depthstencilop_desc() {}
				depthstencilop_desc(gpu_stencilop_type failOp, gpu_stencilop_type depthFailOp, gpu_stencilop_type passOp, gpu_comparisonfunc_type stencilFunc) : StencilFailOp(failOp), StencilDepthFailOp(depthFailOp), StencilPassOp(passOp), StencilFunc(stencilFunc) {}
				gpu_stencilop_type StencilFailOp = gpu_stencilop_type::Keep;
				gpu_stencilop_type StencilDepthFailOp = gpu_stencilop_type::Keep;
				gpu_stencilop_type StencilPassOp = gpu_stencilop_type::Keep;
				gpu_comparisonfunc_type StencilFunc = gpu_comparisonfunc_type::Always;
			};
			depthstencilop_desc FrontFace;
			depthstencilop_desc BackFace;
		} DepthStencilState;



		gpu_tristripcutvalue_type IBStripCutValue = gpu_tristripcutvalue_type::IbscvDisabled;

		// default is no multisampling
		u32 MultisampleCount = 1;
		u32 MultisampleQuality = 0;

		// default is first device node and write to all samples
		u32 NodeMask = CFC_GPU_NODEMASK0();
		u32 SampleMask = 0xFFFFFFFF;
	};

	struct gpu_rootsignature_desc
	{
		enum flags
		{
			FlagNone = 0x0,
			FlagAllowIAInputLayout = 0x1,			// allow input assembler input layout
			FlagDenyVSAccess = 0x2,					// deny vertex shader root access
			FlagDenyHSAccess = 0x4,					// deny hull shader root access
			FlagDenyDSAccess = 0x8,					// deny domain shader root access
			FlagDenyGSAccess = 0x10,				// deny geometry shader root access
			FlagDenyPSAccess = 0x20,				// deny pixel shader root access
			FlagAllowStreamOut = 0x40,				// allow stream out
		};

		enum parameter_type
		{
			RptDescriptorTable = 0,		// descriptor table
			Rpt32BitCsts,				// 32 bit constants
			RptCbv,						// constant buffer view
			RptSrv,						// shader resource view
			RptUav,						// unordered access view
		};

		enum descriptorrange_type
		{
			DrtSrv = 0,					// shader resource view(s)
			DrtUav,						// unordered access view(s)
			DrtCbv,						// constant buffer view(s)
			DrtSampler					// sampler(s)
		};

		struct staticsampler
		{
			static staticsampler Create(u32 shaderRegister = 0, gpu_samplerfilter_type filter = gpu_samplerfilter_type::MinMagMipPoint, gpu_texaddr_type addrMode = gpu_texaddr_type::Clamp, gpu_shadervisibility_type vis = gpu_shadervisibility_type::All) { staticsampler ret; ret.ShaderRegister = shaderRegister; ret.AddressU = ret.AddressV = ret.AddressW = addrMode; ret.Filter = filter; ret.ShaderVisibility = vis; return ret; }
			gpu_samplerfilter_type Filter = gpu_samplerfilter_type::MinMagMipPoint;
			gpu_texaddr_type AddressU = gpu_texaddr_type::Clamp;
			gpu_texaddr_type AddressV = gpu_texaddr_type::Clamp;
			gpu_texaddr_type AddressW = gpu_texaddr_type::Clamp;
			float MipLODBias = 0.0f;
			u32 MaxAnisotropy = 16;
			gpu_comparisonfunc_type ComparisonFunc = gpu_comparisonfunc_type::Never;
			gpu_staticbordercolor_type BorderColor = gpu_staticbordercolor_type::TransparentBlack;
			float MinLOD = 0.0f;
			float MaxLOD = (3.402823466e+38f);
			u32 ShaderRegister = 0;
			u32 RegisterSpace = 0;
			gpu_shadervisibility_type ShaderVisibility = gpu_shadervisibility_type::All;
		};

		struct parameter_range
		{
			parameter_range() {}
			parameter_range(descriptorrange_type type, u32 numDescriptors, u32 baseShaderRegister, u32 registerSpace = 0, u32 offsetInDescriptorsFromTableStart = ~(0U)) :
				Type(type), NumDescriptors(numDescriptors), BaseShaderRegister(baseShaderRegister), RegisterSpace(registerSpace), OffsetInDescriptorsFromTableStart(offsetInDescriptorsFromTableStart) {}

			descriptorrange_type Type = DrtSrv;
			u32 NumDescriptors = 1;
			u32 BaseShaderRegister = 0;
			u32 RegisterSpace = 0;
			u32 OffsetInDescriptorsFromTableStart = ~(0U);
		};

		struct parameter
		{
			parameter_type Type = RptDescriptorTable;								// type
			u32 CstsDescShaderRegister = 0;											// shader register (cX/sX/uX/tX),  unused for RptDescriptorTable (descriptor tables), others uses parameter.
			u32 CstsDescRegisterSpace = 0;											// register namespace (usually 0), unused for RptDescriptorTable (descriptor tables), others uses parameter.
			u32 CstsNum32BitValues = 0;												// number of 32 bit values in constants, only used for Rpt32BitCsts (32 bit constants)
			stl_vector<parameter_range> DescriptorTable_Ranges;						// ranges that are described in descriptor table, only used for RptDescriptorTable
			gpu_shadervisibility_type ShaderVisibility = gpu_shadervisibility_type::All;	// shader visibility 

			static parameter Constants(u32 shaderRegister, u32 num32BitValues, gpu_shadervisibility_type vis = gpu_shadervisibility_type::All, u32 registerSpace = 0) { parameter ret; ret.Type = Rpt32BitCsts; ret.CstsDescShaderRegister = shaderRegister; ret.CstsDescRegisterSpace = registerSpace; ret.CstsNum32BitValues = num32BitValues; ret.ShaderVisibility = vis;  return ret; }
			static parameter SRV(u32 shaderRegister, gpu_shadervisibility_type vis = gpu_shadervisibility_type::All, u32 registerSpace = 0) { parameter ret; ret.Type = RptSrv; ret.CstsDescShaderRegister = shaderRegister; ret.CstsDescRegisterSpace = registerSpace; ret.ShaderVisibility = vis; return ret; }
			static parameter CBV(u32 shaderRegister, gpu_shadervisibility_type vis = gpu_shadervisibility_type::All, u32 registerSpace = 0) { parameter ret; ret.Type = RptCbv; ret.CstsDescShaderRegister = shaderRegister; ret.CstsDescRegisterSpace = registerSpace; ret.ShaderVisibility = vis; return ret; }
			static parameter UAV(u32 shaderRegister, gpu_shadervisibility_type vis = gpu_shadervisibility_type::All, u32 registerSpace = 0) { parameter ret; ret.Type = RptUav; ret.CstsDescShaderRegister = shaderRegister; ret.CstsDescRegisterSpace = registerSpace; ret.ShaderVisibility = vis;  return ret; }
			static parameter DescriptorTable(const parameter_range* ranges, int numRanges, gpu_shadervisibility_type vis = gpu_shadervisibility_type::All) { parameter ret; ret.Type = RptDescriptorTable; for (int i = 0; i < numRanges; i++) ret.DescriptorTable_Ranges.push_back(ranges[i]); ret.ShaderVisibility = vis; return ret; }
		};

		stl_vector<parameter> Parameters;
		stl_vector<staticsampler> StaticSamplers;
		u32 Flags = FlagAllowIAInputLayout;
	};

	struct gpu_computepipelinestate_desc
	{
		gpu_computepipelinestate_desc() {}
		gpu_computepipelinestate_desc(usize rootSignatureIdx, usize computeShaderIdx) : RootSignatureIdx(rootSignatureIdx), ComputeShaderIdx(computeShaderIdx) { }

		// root signature is mandatory - creation will fail with an invalid index.
		usize RootSignatureIdx = cfc::invalid_index;

		// shaders are optional.
		usize ComputeShaderIdx = cfc::invalid_index;

		// default is first device node
		u32 NodeMask = CFC_GPU_NODEMASK0();
	};
#define RESOURCEBARRIER_ALL_SUBRESOURCES  0xFFFFFFFF

	struct CFC_API gpu_resourcebarrier_desc
	{
		struct transitionbarrier_desc
		{
			usize ResourceIdx;
			u32 Subresource;
			gpu_resourcestate::flag StateBefore;
			gpu_resourcestate::flag StateAfter;
		};

		struct transitionaliasing_desc
		{
			usize ResourceBeforeIdx;
			usize ResourceAfterIdx;
		};

		struct transitionuav_desc
		{
			usize ResourceIdx;
		};

		static gpu_resourcebarrier_desc Transition(usize resourceIdx, gpu_resourcestate::flag stateBefore, gpu_resourcestate::flag stateAfter, u32 subResource = RESOURCEBARRIER_ALL_SUBRESOURCES, gpu_resourcebarrierflag_type flags = gpu_resourcebarrierflag_type::None);
		static gpu_resourcebarrier_desc Aliasing(usize resourceBeforeIdx, usize resourceAfterIdx, gpu_resourcebarrierflag_type flags = gpu_resourcebarrierflag_type::None);
		static gpu_resourcebarrier_desc UAV(usize resourceIdx, gpu_resourcebarrierflag_type flags = gpu_resourcebarrierflag_type::None);

		gpu_resourcebarrier_type Type;
		gpu_resourcebarrierflag_type Flags;
		union
		{
			transitionbarrier_desc BarrierTransition;
			transitionaliasing_desc BarrierAliasing;
			transitionuav_desc BarrierUAV;
		};
	};

	struct gpu_defaultclear_desc
	{
		gpu_defaultclear_desc(gpu_format_type fmt = gpu_format_type::Unknown) : Format(fmt) {}
		gpu_format_type Format;
		float ClearDepth = 1.0f;
		u32 ClearStencil = 0;
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	struct gpu_sampler_desc
	{
		gpu_samplerfilter_type Filter = gpu_samplerfilter_type::MinMagMipPoint;
		gpu_texaddr_type AddressU = gpu_texaddr_type::Clamp;
		gpu_texaddr_type AddressV = gpu_texaddr_type::Clamp;
		gpu_texaddr_type AddressW = gpu_texaddr_type::Clamp;
		float MipLODBias = 0.0f;
		u32 MaxAnisotropy = 16;
		gpu_comparisonfunc_type ComparisonFunc = gpu_comparisonfunc_type::Never;
		float BorderColor[4] = { 0.0, 0.0, 0.0, 0.0 };
		float MinLOD = 0.0f;
		float MaxLOD = (3.402823466e+38f);
	};

	struct CFC_API gpu_resource_desc
	{
		enum class dimension
		{
			Unknown,
			Buffer,
			Texture1D,
			Texture2D,
			Texture3D
		};

		enum class texturelayout
		{
			Unknown = 0,
			RowMajor = 1,
			T64KBUndefinedSwizzle = 2,
			T64KBStandardSwizzle = 3
		};

		struct resourceflags
		{
			enum eFlag
			{
				None = 0,
				AllowRenderTarget = 0x1,
				AllowDepthStencil = 0x2,
				AllowUnorderedAccess = 0x4,
				DenyShaderResource = 0x8,
				AllowCrossAdapter = 0x10,
				AllowSimultaneousAccess = 0x20
			};
			typedef u32 flag;
		};

		gpu_resource_desc();
		gpu_resource_desc(dimension a_dimension, u64 alignment, u64 width, u32 height, u16 depth, u16 mipLevels, gpu_format_type a_format, u32 sampleCount, u32 sampleQuality, texturelayout layout, resourceflags::flag flags);
		static gpu_resource_desc Buffer(u64 width, resourceflags::flag flags = resourceflags::None, u64 alignment = 0);
		static gpu_resource_desc Tex1D(gpu_format_type fmt, u64 width, u16 arraySize = 1, u16 mipLevels = 1, resourceflags::flag flags = resourceflags::None, texturelayout layout = texturelayout::Unknown, u64 alignment = 0);
		static gpu_resource_desc Tex2D(gpu_format_type fmt, u64 width, u32 height, u16 arraySize = 1, u16 mipLevels = 1, resourceflags::flag flags = resourceflags::None, texturelayout layout = texturelayout::Unknown, u64 alignment = 0);
		static gpu_resource_desc Tex3D(gpu_format_type fmt, u64 width, u32 height, u16 depth, u16 mipLevels = 1, resourceflags::flag flags = resourceflags::None, texturelayout layout = texturelayout::Unknown, u64 alignment = 0);

		dimension Dimension = dimension::Unknown;
		u64 Alignment = 0;
		u64 Width = 0;
		u32 Height = 0;
		u16 DepthOrArraySize = 1;
		u16 MipLevels = 1;
		gpu_format_type Format = gpu_format_type::Unknown;
		u32 SampleCount = 1;
		u32 SampleQuality = 0;
		texturelayout Layout = texturelayout::Unknown;
		resourceflags::flag Flags = resourceflags::None;
	};

	struct gpu_vertexbuffer_view
	{
		gpu_vertexbuffer_view(usize gpuBufferLocation = cfc::invalid_index, u32 sizeInBytes = 0, u32 strideInBytes = 0) : GpuBufferLocation(gpuBufferLocation), SizeInBytes(sizeInBytes), StrideInBytes(strideInBytes) {}
		u64 GpuBufferLocation;
		u32 SizeInBytes;
		u32 StrideInBytes;
	};

	struct gpu_streamout_view
	{
		gpu_streamout_view(usize gpuBufferLocation = cfc::invalid_index, u64 sizeInBytes = 0, usize gpuBufferFilledSizeLocation = 0) : GpuBufferLocation(gpuBufferLocation), SizeInBytes(sizeInBytes), GpuBufferFilledSizeLocation(gpuBufferFilledSizeLocation) {}
		u64 GpuBufferLocation;
		u64 SizeInBytes;
		usize GpuBufferFilledSizeLocation;
	};

	struct gpu_device_desc
	{
		bool IsHardware = true;
		stl_string Description;
		int DeviceID = -1;
		int VendorID = -1;

		u64 SharedSystemMemory = 0ULL;
		u64 DedicatedSystemMemory = 0ULL;
		u64 DedicatedVideoMemory = 0ULL;
	};

	struct gpu_copyablefootprint_desc
	{
		// base offset
		u64 Offset;

		// footprint data
		gpu_format_type Format;
		u32 Width;
		u32 Height;
		u32 Depth;
		u32 RowPitch;

		// extra information from GetCopyableFootprints
		u32 NumRows;
		u64 RowSizeInBytes;
		u64 TotalBytes;
	};

	struct gpu_shaderreflection_desc
	{
		enum resource_type
		{
			ConstantBuffer = 0,
			TextureBuffer,
			Texture,
			Sampler,
			UavRwTyped,
			Structured,
			UavRwStructured,
			Byteaddress,
			UavRwByteaddress,
			UavAppendStructured,
			UavConsumeStructured,
			UavRwStructuredWithCounter,
		};

		enum class return_type
		{
			Unorm = 1,
			Snorm = 2,
			Sint = 3,
			Uint = 4,
			Float = 5,
			Mixed = 6,
			Double = 7,
			Continued = 8,
		};

		enum class dimension_type
		{
			Unknown = 0,
			Buffer = 1,
			Tex1D = 2,
			Tex1DArray = 3,
			Tex2D = 4,
			Tex2DArray = 5,
			Tex2DMS = 6,
			Tex2DMSArray = 7,
			Tex3D = 8,
			TexCube = 9,
			TexCubeArray = 10,
			BufferEx = 11,
		};

		enum class component_type
		{
			Unknown = 0,
			Uint32 = 1,
			Sint32 = 2,
			Float32 = 3,
		};

		enum class minprecision_type
		{
			Default = 0,
			Float16 = 1,
			Float2_8 = 2,
			Reserved = 3,
			Sint16 = 4,
			Uint16 = 5,
			Any16 = 0xf0,
			Any10 = 0xf1
		};

		enum class variable_type
		{
			Unknown = 0,
			Void,
			Float,
			Double,
			Half,
			Int32,
			Uint32,
			Int16,
			Uint16,
			Int8,
			Uint8,
		};

		struct CFC_API parameter_desc
		{
			stl_string					 SemanticName;   // Name of the semantic
			u32							 SemanticIndex;  // Index of the semantic
			u32							 Register;       // Number of member variables
			u32							 SystemValueType;// A predefined system value, or D3D_NAME_UNDEFINED if not applicable
			component_type				 ComponentType;  // Scalar type (e.g. uint, float, etc.)
			u8							 Mask;           // Mask to indicate which components of the register are used.
			u8							 ReadWriteMask;  // Mask to indicate whether a given component is never written (if this is an output signature) or always read (if this is an input signature).
			u32							 Stream;         // Stream index
			minprecision_type			 MinPrecision;   // Minimum desired interpolation precision

			u32 ConvertMaskToNumChannels() const;
		};

		struct gpu_resource_desc
		{
			stl_string                  Name;           // Name of the resource
			resource_type				Type;           // Type of resource (e.g. texture, cbuffer, etc.)
			u32							TypeIndex = ~0; // Detailed type data index (only for cbuffers)
			u32                         BindPoint;      // Starting bind point
			u32                         BindCount;      // Number of contiguous bind points (for arrays)

			u32                         uFlags;         // Input binding flags
			return_type				    ReturnType;     // Return type (if texture)
			dimension_type				Dimension;      // Dimension (if texture)
			u32							NumSamples;     // Number of samples (0 if not MS texture)
			u32							Space;          // Register space
			u32							uID;	        // Range ID in the bytecode
		};



		struct cbuffer_var_desc
		{
			stl_string					Name;
			u32							Offset = ~0;
			u32							SizeInBytes = ~0;

			variable_type				Type = variable_type::Unknown;
			u32							TypeElements = 1;	// for arrays..
			u32							TypeRows = 1;		// for matrix types..
			u32							TypeColumns = 1;	// for matrix types..
		};

		struct cbuffer_desc
		{
			u32							ResourceIndex;  // Original resource index
			u32							SizeInBytes;	// Number of bytes for the entire cbuffer
			stl_vector<cbuffer_var_desc> Variables;		// Variable descriptions
		};

		struct info_desc
		{
			u32						ConstantBuffers = ~0;            // Number of constant buffers
			u32						BoundResources = ~0;             // Number of bound resources
			u32						InputParameters = ~0;            // Number of parameters in the input signature
			u32						OutputParameters = ~0;           // Number of parameters in the output signature

			u32						InstructionCount = ~0;           // Number of emitted instructions
			u32						TempRegisterCount = ~0;          // Number of temporary registers used 
			u32						TempArrayCount = ~0;             // Number of temporary arrays used
			u32						DefCount = ~0;                   // Number of constant defines 
			u32						DclCount = ~0;                   // Number of declarations (input + output)

			u32						ComputeThreadGroupX = ~0;		// Thread group X size
			u32						ComputeThreadGroupY = ~0;		// Thread group Y size
			u32						ComputeThreadGroupZ = ~0;		// Thread group Z size
		};

		info_desc					Info;
		stl_vector<gpu_resource_desc>	Resources;
		stl_vector<cbuffer_desc>	Cbuffers;
		stl_vector<parameter_desc>	Inputs;
		stl_vector<parameter_desc>	Outputs;
	};

	struct CFC_API gpu_texturecopy_desc
	{
		static gpu_texturecopy_desc AsPlacedFootprint(usize idx, const gpu_copyablefootprint_desc& desc);
		static gpu_texturecopy_desc AsSubresourceIndex(usize idx, u32 subresourceIndex = 0);

		enum class copytype
		{
			SubresourceIndex,
			PlacedFootprint,
		};
		usize ResourceIdx = cfc::invalid_index;
		copytype Type;
		u32 SubresourceIndex;
		gpu_copyablefootprint_desc PlacedFootprint;
	};

	struct CFC_API gpu_features
	{
		enum class crossnode_type
		{
			None = 0,                         // No cross node support.
			Tier1Emulated = 1,                // Tier 1 emulation is supported (copy through CPU).
			Tier1 = 2,                        // Tier 1 is supported (Copy[BufferRegion/TextureRegion/Resource] across nodes)
			Tier2 = 3                         // Tier 1 + all other cross node features are supported. Does not support RTV/DSV/UAV atomic operations across nodes.
		};

		enum class minprecision_type
		{
			None = 0,                        // The driver supports only full 32-bit precision for all shader stages.
			TenBit = 0x1,                    // The driver supports 10-bit precision.
			SixteenBit = 0x2                 // The driver supports 16-bit precision.
		};

		enum class tiledresources_type
		{
			None = 0,                         // Unsupported.
			Tier1 = 1,                        // Supported, but GPU reads or writes to NULL mappings are undefined. (map the same page to everywhere a NULL mapping would've been used)
			Tier2 = 2,                        // When the size of a texture mipmap level is at least one standard tile shape for its format, the mipmap level is guaranteed to be nonpacked. Shader instructions are available for clamping level - of - detail(LOD) and for obtaining status about the shader operation. Reading from NULL - mapped tiles treat that sampled value as zero.Writes to NULL - mapped tiles are discarded.
			Tier3 = 3                         // 3D textures are also supported.
		};

		enum class conservativerst_type
		{
			None = 0,                         // Unsupported.
			Tier1 = 1,                        // Tier 1 enforces a maximum 1/2 pixel uncertainty region and does not support post-snap degenerates. This is good for tiled rendering, a texture atlas, light map generation and sub-pixel shadow maps.
			Tier2 = 2,                        // Tier 2 reduces the maximum uncertainty region to 1/256 and requires post-snap degenerates not be culled. This tier is helpful for CPU-based algorithm acceleration (such as voxelization).
			Tier3 = 3                         // Tier 3 maintains a maximum 1/256 uncertainty region and adds support for inner input coverage. Inner input coverage adds the new value SV_InnerCoverage to High Level Shading Language (HLSL). This is a 32-bit scalar integer that can be specified on input to a pixel shader, and represents the underestimated conservative rasterization information (that is, whether a pixel is guaranteed-to-be-fully covered). This tier is helpful for occlusion culling.
		};

		enum class resourcebinding_type
		{
			Tier1 = 1,                        // 1M CBV/UAV/SRV heaps, 14 CBVs per stage, 128 SRVs per stage, 16 samplers per stage & 64/8 UAVs across all stages
			Tier2 = 2,                        // 1M CBV/UAV/SRV heaps, 14 CBVs per stage, HEAP SRVs per stage, HEAP samplers per stage & 64 UAVs across all stages
			Tier3 = 3                         // 1M+ CBV/UAV/SRV heaps, HEAP CBVs per stage, HEAP SRVs per stage, HEAP samplers per stage & HEAP UAVs across all stages
		};

		enum class resourceheap_type
		{
			Tier1 = 1,
			Tier2 = 2
		};

		u32                    DoublePrecisionFloatShaderOps;
		u32                    OutputMergerLogicOp;
		minprecision_type      MinPrecisionSupport;
		tiledresources_type    TiledResourcesTier;
		resourcebinding_type   ResourceBindingTier;
		u32                    PSSpecifiedStencilRefSupported;
		u32                    TypedUAVLoadAdditionalFormats;
		u32                    ROVsSupported;
		conservativerst_type   ConservativeRasterizationTier;
		u32                    MaxGPUVirtualAddressBitsPerResource;
		u32                    StandardSwizzle64KBSupported;
		crossnode_type         CrossNodeSharingTier;
		u32                    CrossAdapterRowMajorTextureSupported;
		u32                    VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation;
		resourceheap_type      ResourceHeapTier;
	};

	struct gpu_viewport
	{
		gpu_viewport(f32 x = 0, f32 y = 0, f32 w = 0, f32 h = 0, f32 dMin = 0.0f, f32 dMax = 1.0f) : TopLeftX(x), TopLeftY(y), Width(w), Height(h), MinDepth(dMin), MaxDepth(dMax) {}
		f32 TopLeftX;
		f32 TopLeftY;
		f32 Width;
		f32 Height;
		f32 MinDepth;
		f32 MaxDepth;
	};

	struct gpu_rectangle
	{
		gpu_rectangle(long x = 0, long y = 0, long x2 = 0, long y2 = 0) : X(x), Y(y), X2(x2), Y2(y2) {}
		long X;
		long Y;
		long X2;
		long Y2;
	};

	struct gpu_box
	{
		gpu_box(u32 x = 0, u32 y = 0, u32 z = 0, u32 x2 = 0, u32 y2 = 0, u32 z2 = 0) : X(x), Y(y), Z(z), X2(x2), Y2(y2), Z2(z2) {}
		u32 X;
		u32 Y;
		u32 Z;
		u32 X2;
		u32 Y2;
		u32 Z2;
	};



}; // end namespace cfc
