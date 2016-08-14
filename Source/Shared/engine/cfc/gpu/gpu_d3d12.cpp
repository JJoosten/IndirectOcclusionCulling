// ** CONFIGURATION
// Enable Multi-GPU support
#define CFC_DX12_MGPU_AFFINITY
// Enable D3D debug layer support
//#define CFC_DX12_ENABLE_DEBUG

// ** END CONFIGURATION

#ifdef CFC_DX12_MGPU_AFFINITY
#define CFC_DX12_MGPU_ADDAFFINITYMASK ,AffinityMask
#else
#define CFC_DX12_MGPU_ADDAFFINITYMASK 
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#ifdef CFC_DX12_MGPU_AFFINITY
#include <dependencies/d3dxaffinity/d3dx12affinity.h>
#include <dependencies/d3dxaffinity/d3dx12affinity_d3dx12.h>
typedef CD3DX12AffinityObject							ID3D12ProxyObject;
typedef CD3DX12AffinityDeviceChild						ID3D12ProxyDeviceChild;
typedef CD3DX12AffinityPageable							ID3D12ProxyPageable;
typedef CD3DX12AffinityRootSignature					ID3D12ProxyRootSignature;
typedef CD3DX12AffinityRootSignatureDeserializer		ID3D12ProxyRootSignatureDeserializer;
typedef CD3DX12AffinityHeap								ID3D12ProxyHeap;
typedef CD3DX12AffinityResource							ID3D12ProxyResource;
typedef CD3DX12AffinityCommandAllocator					ID3D12ProxyCommandAllocator;
typedef CD3DX12AffinityFence							ID3D12ProxyFence;
typedef CD3DX12AffinityPipelineState					ID3D12ProxyPipelineState;
typedef CD3DX12AffinityDescriptorHeap					ID3D12ProxyDescriptorHeap;
typedef CD3DX12AffinityQueryHeap						ID3D12ProxyQueryHeap;
typedef CD3DX12AffinityCommandSignature					ID3D12ProxyCommandSignature;
typedef CD3DX12AffinityCommandList						ID3D12ProxyCommandList;
typedef CD3DX12AffinityGraphicsCommandList				ID3D12ProxyGraphicsCommandList;
typedef CD3DX12AffinityCommandQueue						ID3D12ProxyCommandQueue;
typedef CD3DX12AffinityDevice							ID3D12ProxyDevice;
typedef CDXGIAffinitySwapChain							IDXGIProxySwapChain;

typedef D3DX12_AFFINITY_GRAPHICS_PIPELINE_STATE_DESC	D3D12_PROXY_GRAPHICS_PIPELINE_STATE_DESC;
typedef D3DX12_AFFINITY_COMPUTE_PIPELINE_STATE_DESC		D3D12_PROXY_COMPUTE_PIPELINE_STATE_DESC;
typedef D3DX12_AFFINITY_RESOURCE_TRANSITION_BARRIER		D3D12_PROXY_RESOURCE_TRANSITION_BARRIER;
typedef D3DX12_AFFINITY_RESOURCE_ALIASING_BARRIER		D3D12_PROXY_RESOURCE_ALIASING_BARRIER;
typedef D3DX12_AFFINITY_RESOURCE_UAV_BARRIER			D3D12_PROXY_RESOURCE_UAV_BARRIER;
typedef D3DX12_AFFINITY_RESOURCE_BARRIER				D3D12_PROXY_RESOURCE_BARRIER;
typedef D3DX12_AFFINITY_TEXTURE_COPY_LOCATION			D3D12_PROXY_TEXTURE_COPY_LOCATION;

typedef CD3DX12_AFFINITY_TEXTURE_COPY_LOCATION			CD3DX12_PROXY_TEXTURE_COPY_LOCATION;
typedef CD3DX12_AFFINITY_RESOURCE_DESC					CD3DX12_PROXY_RESOURCE_DESC;
typedef CD3DX12_AFFINITY_RESOURCE_BARRIER				CD3DX12_PROXY_RESOURCE_BARRIER;

typedef	ID3D12ProxyCommandSignature						ID3D12ProxyCommandSignatureOut;
#else
#include <d3d12.h>
#include <dependencies/d3dx12.h>
typedef ID3D12Object 									ID3D12ProxyObject;
typedef ID3D12DeviceChild 								ID3D12ProxyDeviceChild;
typedef ID3D12Pageable 									ID3D12ProxyPageable;
typedef ID3D12RootSignature 							ID3D12ProxyRootSignature;
typedef ID3D12RootSignatureDeserializer					ID3D12ProxyRootSignatureDeserializer;
typedef ID3D12Heap 										ID3D12ProxyHeap;
typedef ID3D12Resource 									ID3D12ProxyResource;
typedef ID3D12CommandAllocator 							ID3D12ProxyCommandAllocator;
typedef ID3D12Fence 									ID3D12ProxyFence;
typedef ID3D12PipelineState 							ID3D12ProxyPipelineState;
typedef ID3D12DescriptorHeap 							ID3D12ProxyDescriptorHeap;
typedef ID3D12QueryHeap									ID3D12ProxyQueryHeap;
typedef ID3D12CommandSignature 							ID3D12ProxyCommandSignature;
typedef ID3D12CommandList 								ID3D12ProxyCommandList;
typedef ID3D12GraphicsCommandList 						ID3D12ProxyGraphicsCommandList;
typedef ID3D12CommandQueue 								ID3D12ProxyCommandQueue;
typedef ID3D12Device									ID3D12ProxyDevice;
typedef	void											ID3D12ProxyCommandSignatureOut;

typedef IDXGISwapChain									IDXGIProxySwapChain;

typedef D3D12_GRAPHICS_PIPELINE_STATE_DESC				D3D12_PROXY_GRAPHICS_PIPELINE_STATE_DESC;
typedef D3D12_COMPUTE_PIPELINE_STATE_DESC				D3D12_PROXY_COMPUTE_PIPELINE_STATE_DESC;
typedef D3D12_RESOURCE_TRANSITION_BARRIER				D3D12_PROXY_RESOURCE_TRANSITION_BARRIER;
typedef D3D12_RESOURCE_ALIASING_BARRIER					D3D12_PROXY_RESOURCE_ALIASING_BARRIER;
typedef D3D12_RESOURCE_UAV_BARRIER						D3D12_PROXY_RESOURCE_UAV_BARRIER;
typedef D3D12_RESOURCE_BARRIER							D3D12_PROXY_RESOURCE_BARRIER;
typedef D3D12_TEXTURE_COPY_LOCATION						D3D12_PROXY_TEXTURE_COPY_LOCATION;

typedef CD3DX12_TEXTURE_COPY_LOCATION					CD3DX12_PROXY_TEXTURE_COPY_LOCATION;
typedef CD3DX12_RESOURCE_DESC							CD3DX12_PROXY_RESOURCE_DESC;
typedef CD3DX12_RESOURCE_BARRIER						CD3DX12_PROXY_RESOURCE_BARRIER;
#endif

#include <dxgi1_4.h>
#include <D3Dcompiler.h>

#include <DirectXMath.h>

#include <wrl.h>
#include <shellapi.h>

#include <cfc/gpu/gpu_d3d12.h>
#include <cfc/core/window.h>
#include <mutex>
#include <cfc/stl/stl_unique_ptr.hpp>
#include <cfc/stl/stl_string.hpp>
#include <cfc/stl/stl_string_advanced.hpp>
#include <cfc/stl/stl_threading.hpp>
#include <cfc/stl/stl_resource_collection.hpp>

#define ThrowIfFailed(x) { if(FAILED(x)) { CFC_BREAKPOINT; } }

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace cfc {

#define U64_GET_LOW_WORD(x) (x & (u64)0xFFFFFFFF)
#define U64_GET_HIGH_WORD(x) (x >> (u64)32)

// windows event wrapper for automatic cleanup of events
struct _imp_dx12_fence_event
{
	_imp_dx12_fence_event() { ev = CreateEventA(nullptr, FALSE, FALSE, nullptr); }
	~_imp_dx12_fence_event() { CloseHandle(ev); }
	HANDLE ev;
};

struct _imp_dx12_shader
{
	ComPtr<ID3DBlob> blob;
	cfc::gpu_shaderreflection_desc reflected;
};

struct _imp_dx12_resource
{
	~_imp_dx12_resource() 
	{ 
		if (custom)
			free(custom); 
	}
	ComPtr<ID3D12ProxyResource> resource;
	cfc::gpu_heap_type heapType;
	cfc::gpu_resource_desc desc;
	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress;
	void* custom = nullptr;
	
};

struct _imp_gpu_dx12_context
{
	ComPtr<IDXGIFactory4> factory;
	ComPtr<ID3D12ProxyDevice> device;
	ComPtr<IDXGIAdapter> adapter;

	stl_resource_collection<ComPtr<IDXGIProxySwapChain>, stl_mutex> swapChains;
	stl_resource_collection<ComPtr<ID3D12ProxyCommandQueue>, stl_mutex> commandQueues;
	stl_resource_collection<ComPtr<ID3D12ProxyCommandAllocator>, stl_mutex> commandAllocators;
	stl_resource_collection<ComPtr<ID3D12ProxyCommandList>, stl_mutex> commandLists;
	stl_resource_collection<ComPtr<ID3D12ProxyCommandSignature>, stl_mutex> commandSignatures;
	stl_resource_collection<ComPtr<ID3D12ProxyFence>, stl_mutex> fences;
	stl_resource_collection<stl_unique_ptr<_imp_dx12_fence_event>, stl_mutex> fenceEvents;
	stl_resource_collection<ComPtr<ID3D12ProxyQueryHeap>, stl_mutex> queryHeaps;
	stl_resource_collection<ComPtr<ID3D12ProxyDescriptorHeap>, stl_mutex> descriptorHeaps;
	stl_resource_collection<ComPtr<ID3D12ProxyRootSignature>, stl_mutex> rootSignatures;
	stl_resource_collection<_imp_dx12_shader, stl_mutex> shaderBlobs;
	stl_resource_collection<ComPtr<ID3D12ProxyPipelineState>, stl_mutex> pipelineStates;
	stl_resource_collection<_imp_dx12_resource, stl_mutex> resources;

};

struct _imp_gpu_dx12_indirect
{
	stl_vector<D3D12_INDIRECT_ARGUMENT_DESC> argumentDescriptor;
	usize commandSignatureIdx;
	u32 numArgumentsInDescriptor;
	bool needsAlignedStructPadding;
};

#pragma region Converters

static D3D12_COMMAND_LIST_TYPE Convert(gpu_commandlist_type type)
{
	D3D12_COMMAND_LIST_TYPE ret;
	switch (type)
	{
		case gpu_commandlist_type::Direct:			ret = D3D12_COMMAND_LIST_TYPE_DIRECT; break;
		case gpu_commandlist_type::Copy:				ret = D3D12_COMMAND_LIST_TYPE_COPY; break;
		case gpu_commandlist_type::Bundle:			ret = D3D12_COMMAND_LIST_TYPE_BUNDLE; break;
		case gpu_commandlist_type::Compute:			ret = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
	}
	return ret;
}

static D3D12_DESCRIPTOR_HEAP_TYPE Convert(gpu_descriptorheap_type type)
{
	D3D12_DESCRIPTOR_HEAP_TYPE ret;
	switch (type)
	{
		case gpu_descriptorheap_type::CbvSrvUav:		ret = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; break;
		case gpu_descriptorheap_type::Dsv:			ret = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; break;
		case gpu_descriptorheap_type::Rtv:			ret = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; break;
		case gpu_descriptorheap_type::Samplers:		ret = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER; break;
	}
	return ret;
}

static D3D12_QUERY_HEAP_TYPE Convert(gpu_queryheap_type type)
{
	D3D12_QUERY_HEAP_TYPE ret;
	switch (type)
	{
	case gpu_queryheap_type::Occlusion:		ret = D3D12_QUERY_HEAP_TYPE_OCCLUSION; break;
	case gpu_queryheap_type::TimeStamp:		ret = D3D12_QUERY_HEAP_TYPE_TIMESTAMP; break;
	}
	return ret;
}

static D3D12_HEAP_TYPE Convert(gpu_heap_type type)
{
	D3D12_HEAP_TYPE ret;
	switch (type)
	{
		case gpu_heap_type::Default:		ret = D3D12_HEAP_TYPE_DEFAULT; break;
		case gpu_heap_type::Upload:			ret = D3D12_HEAP_TYPE_UPLOAD; break;
		case gpu_heap_type::Readback:		ret = D3D12_HEAP_TYPE_READBACK; break;
		case gpu_heap_type::Custom:			ret = D3D12_HEAP_TYPE_CUSTOM; break;
	}
	return ret;
}

static const D3D12_SAMPLER_DESC& Convert(const gpu_sampler_desc& desc)
{
	return *(const D3D12_SAMPLER_DESC*)&desc;
}

static const D3D12_RESOURCE_DESC& Convert(const gpu_resource_desc& desc)
{
	return *(const D3D12_RESOURCE_DESC*)&desc;
}

static D3D12_RESOURCE_STATES Convert(gpu_resourcestate::flag flags)
{
	return (D3D12_RESOURCE_STATES)flags;
}

static D3D12_QUERY_TYPE Convert(gpu_query_type type)
{
	return (D3D12_QUERY_TYPE)type;
}

static DXGI_FORMAT Convert(gpu_format_type type)
{
	return (DXGI_FORMAT)type;
}

static gpu_format_type Convert(DXGI_FORMAT type)
{
	return (gpu_format_type)type;
}

static D3D12_INPUT_CLASSIFICATION Convert(gpu_graphicspipelinestate_desc::inputelement_desc::cls_type clstype)
{
	return (D3D12_INPUT_CLASSIFICATION)clstype;
}

static D3D12_DEPTH_WRITE_MASK Convert(gpu_depthwritemask_type t)
{
	return (D3D12_DEPTH_WRITE_MASK)t;
}

static D3D12_COMPARISON_FUNC Convert(gpu_comparisonfunc_type t)
{
	return (D3D12_COMPARISON_FUNC)t;
}

static D3D12_STENCIL_OP Convert(gpu_stencilop_type t)
{
	return (D3D12_STENCIL_OP)t;
}

static D3D12_PRIMITIVE_TOPOLOGY_TYPE Convert(gpu_primitivetopology_type t)
{
	return (D3D12_PRIMITIVE_TOPOLOGY_TYPE)t;
}

static D3D12_INDEX_BUFFER_STRIP_CUT_VALUE Convert(gpu_tristripcutvalue_type t)
{
	return (D3D12_INDEX_BUFFER_STRIP_CUT_VALUE)t;
}

static D3D12_BLEND_OP Convert(gpu_blendop_type op)
{
	return (D3D12_BLEND_OP)op;
}

static D3D12_BLEND Convert(gpu_blendmode_type t)
{
	return (D3D12_BLEND)t;
}

static D3D12_LOGIC_OP Convert(gpu_logicop_type t)
{
	return (D3D12_LOGIC_OP)t;
}

static D3D12_SHADER_VISIBILITY Convert(gpu_shadervisibility_type t)
{
	return (D3D12_SHADER_VISIBILITY)t;
}

static D3D12_FILTER Convert(gpu_samplerfilter_type t)
{
	return (D3D12_FILTER)t;
}

static D3D12_TEXTURE_ADDRESS_MODE Convert(gpu_texaddr_type t)
{
	return (D3D12_TEXTURE_ADDRESS_MODE)t;
}

static D3D12_DESCRIPTOR_RANGE_TYPE Convert(gpu_rootsignature_desc::descriptorrange_type t)
{
	return (D3D12_DESCRIPTOR_RANGE_TYPE)t;
}


static D3D12_RASTERIZER_DESC Convert(const gpu_graphicspipelinestate_desc::rasterizer_desc& rstdesc)
{
	D3D12_RASTERIZER_DESC ret;
	ret.AntialiasedLineEnable	= rstdesc.AntialiasedLineEnable;
	ret.ConservativeRaster		= (D3D12_CONSERVATIVE_RASTERIZATION_MODE)rstdesc.ConservativeRaster;
	ret.CullMode				= (D3D12_CULL_MODE)rstdesc.CullMode;
	ret.DepthBias				= rstdesc.DepthBias;
	ret.DepthBiasClamp			= rstdesc.DepthBiasClamp;
	ret.DepthClipEnable			= rstdesc.DepthClipEnable;
	ret.FillMode				= (D3D12_FILL_MODE)rstdesc.FillMode;
	ret.ForcedSampleCount		= rstdesc.ForcedSampleCount;
	ret.FrontCounterClockwise	= rstdesc.FrontCounterClockwise;
	ret.MultisampleEnable		= rstdesc.MultisampleEnable;
	ret.SlopeScaledDepthBias	= rstdesc.SlopeScaledDepthBias;
	return ret;
}

static D3D12_DEPTH_STENCILOP_DESC Convert(const gpu_graphicspipelinestate_desc::depthstencil_desc::depthstencilop_desc& desc)
{
	D3D12_DEPTH_STENCILOP_DESC ret;
	ret.StencilFunc =			Convert(desc.StencilFunc);
	ret.StencilPassOp =			Convert(desc.StencilPassOp);
	ret.StencilFailOp =			Convert(desc.StencilFailOp);
	ret.StencilDepthFailOp =	Convert(desc.StencilDepthFailOp);
	return ret;
}


static D3D12_DEPTH_STENCIL_DESC Convert(const gpu_graphicspipelinestate_desc::depthstencil_desc& desc)
{
	D3D12_DEPTH_STENCIL_DESC ret;
	ret.DepthEnable =					desc.DepthEnable?TRUE:FALSE;
	ret.DepthWriteMask =		Convert(desc.DepthWriteMask);
	ret.DepthFunc =				Convert(desc.DepthFunc);
	ret.StencilEnable =					desc.StencilEnable?TRUE:FALSE;
	ret.StencilReadMask =				desc.StencilReadMask;
	ret.StencilWriteMask =				desc.StencilWriteMask;
	ret.FrontFace =				Convert(desc.FrontFace);
	ret.BackFace =				Convert(desc.BackFace);
	return ret;
}

static D3D12_INPUT_ELEMENT_DESC Convert(const gpu_graphicspipelinestate_desc::inputelement_desc& desc)
{
	D3D12_INPUT_ELEMENT_DESC ret;
	ret.AlignedByteOffset =				desc.AlignedByteOffset;
	ret.Format =				Convert(desc.Format);
	ret.InputSlot =						desc.InputSlot;
	ret.InputSlotClass =		Convert(desc.InputSlotClass);
	ret.InstanceDataStepRate =			desc.InstanceDataStepRate;
	ret.SemanticIndex =					desc.SemanticIndex;
	ret.SemanticName =					desc.SemanticName.c_str();
	return ret;
}



static D3D12_RENDER_TARGET_BLEND_DESC Convert(const gpu_graphicspipelinestate_desc::rtblendstate_desc& desc)
{
	D3D12_RENDER_TARGET_BLEND_DESC ret;
	ret.BlendEnable = desc.BlendEnabled ? TRUE : FALSE;
	ret.LogicOpEnable = desc.LogicOpEnabled ? TRUE : FALSE;
	ret.BlendOp = Convert(desc.BlendOp);
	ret.BlendOpAlpha = Convert(desc.BlendOpAlpha);
	ret.DestBlend = Convert(desc.DstBlend);
	ret.DestBlendAlpha = Convert(desc.DstBlendAlpha);
	ret.LogicOp = Convert(desc.LogicOp);
	ret.RenderTargetWriteMask = desc.RenderTargetWriteMask;
	ret.SrcBlend = Convert(desc.SrcBlend);
	ret.SrcBlendAlpha = Convert(desc.SrcBlendAlpha);
	return ret;
}

static D3D12_BLEND_DESC Convert(const gpu_graphicspipelinestate_desc::blendstate_desc& desc)
{
	D3D12_BLEND_DESC ret;
	ret.AlphaToCoverageEnable = desc.AlphaToCoverageEnable ? TRUE : FALSE;
	ret.IndependentBlendEnable = desc.IndependentBlendEnable ? TRUE : FALSE;
	for (int i = 0; i < 8; i++)
		ret.RenderTarget[i] = Convert(desc.RenderTarget[i]);
	return ret;
}

static D3D12_CLEAR_VALUE Convert(const gpu_defaultclear_desc& dclear)
{
	D3D12_CLEAR_VALUE ret;
	for (int i = 0; i < 4; i++)
		ret.Color[i] = dclear.ClearColor[i];
	ret.DepthStencil.Depth = dclear.ClearDepth;
	ret.DepthStencil.Stencil = dclear.ClearStencil;
	ret.Format = Convert(dclear.Format);
	return ret;
}

static D3D12_GPU_DESCRIPTOR_HANDLE ConvertGPU(u64 addr)
{
	D3D12_GPU_DESCRIPTOR_HANDLE ret;
	ret.ptr = addr;
	return ret;
}

static D3D12_CPU_DESCRIPTOR_HANDLE ConvertCPU(usize addr)
{
	D3D12_CPU_DESCRIPTOR_HANDLE ret;
	ret.ptr = addr;
	return ret;
}

static D3D12_PROXY_TEXTURE_COPY_LOCATION Convert(const gpu_texturecopy_desc& desc, ID3D12ProxyResource* res)
{
	D3D12_PROXY_TEXTURE_COPY_LOCATION ret;
	ret.pResource = res;
	if (desc.Type == gpu_texturecopy_desc::copytype::SubresourceIndex)
	{
		ret.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		ret.SubresourceIndex = desc.SubresourceIndex;
	}
	else if(desc.Type == gpu_texturecopy_desc::copytype::PlacedFootprint)
	{
		ret.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		ret.PlacedFootprint.Offset = desc.PlacedFootprint.Offset;
		ret.PlacedFootprint.Footprint.Width = desc.PlacedFootprint.Width;
		ret.PlacedFootprint.Footprint.Height = desc.PlacedFootprint.Height;
		ret.PlacedFootprint.Footprint.Depth = desc.PlacedFootprint.Depth;
		ret.PlacedFootprint.Footprint.Format = Convert(desc.PlacedFootprint.Format);
		ret.PlacedFootprint.Footprint.RowPitch = desc.PlacedFootprint.RowPitch;
	}
	return ret;
}

static gpu_shaderreflection_desc::parameter_desc Convert(const D3D12_SIGNATURE_PARAMETER_DESC& desc)
{
	gpu_shaderreflection_desc::parameter_desc ret;
	ret.ComponentType = (gpu_shaderreflection_desc::component_type)desc.ComponentType;
	ret.Mask = desc.Mask;
	ret.MinPrecision = (gpu_shaderreflection_desc::minprecision_type)desc.MinPrecision;
	ret.ReadWriteMask = desc.ReadWriteMask;
	ret.Register = desc.Register;
	ret.SemanticIndex = desc.SemanticIndex;
	ret.SemanticName = desc.SemanticName;
	ret.Stream = desc.Stream;
	ret.SystemValueType = desc.SystemValueType;
	return ret;
}

static gpu_features Convert(const D3D12_FEATURE_DATA_D3D12_OPTIONS& opts) { gpu_features ret = *(const gpu_features*)&opts; return ret; }
#pragma endregion

gpu_dx12_context::gpu_dx12_context()
{
	m_impl.init();

#if defined(CFC_DX12_ENABLE_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_impl->factory)));


}

gpu_dx12_context::~gpu_dx12_context()
{
	m_impl.destroy();
}

template <class T>
void iterateDevices(IDXGIFactory4* factory, const T& it)
{
	int adapterIndex = 0;
	while(true)
	{
		ComPtr<IDXGIAdapter> pAdapter;

		if (factory->EnumAdapters(adapterIndex++, &pAdapter) == DXGI_ERROR_NOT_FOUND)
			break;

		DXGI_ADAPTER_DESC1 desc1;
		IDXGIAdapter1* pOAdapter = (IDXGIAdapter1*)pAdapter.Get();
		pOAdapter->GetDesc1(&desc1);

		if (desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue; // skip pure software devices

		it((DXGI_ADAPTER_DESC&)desc1, pAdapter.Get(), true);
	}

	// WARP adapter
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		DXGI_ADAPTER_DESC desc;
		if(SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter))))
		{
			pWarpAdapter->GetDesc(&desc);

			// override description
			desc.Description[0] = 'W';
			desc.Description[1] = 'A';
			desc.Description[2] = 'R';
			desc.Description[3] = 'P';
			desc.Description[4] = 0;
			it(desc, pWarpAdapter.Get(), false);
		}
	}
}

usize gpu_dx12_context::DeviceGetCount()
{
	u32_64 count = 0;
	iterateDevices(m_impl->factory.Get(), [&](DXGI_ADAPTER_DESC&, IDXGIAdapter*, bool) { count++; });
	return count;
}

gpu_device_desc gpu_dx12_context::DeviceGetInfo(usize idx)
{
	gpu_device_desc info;
	u32_64 count = 0;
	iterateDevices(m_impl->factory.Get(), [&](DXGI_ADAPTER_DESC& dev, IDXGIAdapter*, bool IsHardware) 
	{
		if (count++ == idx)
		{
			info.DedicatedSystemMemory = dev.DedicatedSystemMemory;
			info.DedicatedVideoMemory = dev.DedicatedVideoMemory;
			info.SharedSystemMemory = dev.SharedSystemMemory;
			info.VendorID = dev.VendorId;
			info.Description = stl_string_advanced::utf8_fromUtf16((u16*)dev.Description,sizeof(dev.Description) / sizeof(dev.Description[0]));
			info.DeviceID = dev.DeviceId;
			info.IsHardware = IsHardware;
		}
	});
	
	return info;
}

bool gpu_dx12_context::DeviceInit(u32_64 deviceIdx)
{
	// get adapter
	
	u32_64 count = 0;
	iterateDevices(m_impl->factory.Get(), [&](DXGI_ADAPTER_DESC& dev, IDXGIAdapter* devAdapter, bool IsHardware) 
	{
		if (count++ == deviceIdx)
			m_impl->adapter = devAdapter;
	});


	// initialize device
	ComPtr<ID3D12Device> device;
	if (FAILED(D3D12CreateDevice(m_impl->adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device))))
		if (FAILED(D3D12CreateDevice(m_impl->adapter.Get(),	D3D_FEATURE_LEVEL_12_0,	IID_PPV_ARGS(&device))))
			if (FAILED(D3D12CreateDevice(m_impl->adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&device))))
				if (FAILED(D3D12CreateDevice(m_impl->adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
					return false;

#ifdef CFC_DX12_MGPU_AFFINITY
	D3DX12AffinityCreateLDADevice(device.Get(), m_impl->device.ReleaseAndGetAddressOf());
#else
	m_impl->device = device;
#endif

	// device initialized
	return true;
}


gpu_features gpu_dx12_context::DeviceGetFeatures() const
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS structure;
	HRESULT result = m_impl->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &structure, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS));

	if (SUCCEEDED(result))
		return Convert(structure);
	else
		return gpu_features();
}

bool gpu_dx12_context::DeviceIsMGPUEnabled() const
{
#ifdef CFC_DX12_MGPU_AFFINITY
	return m_impl->device->GetNodeCount() > 1;
#else
	return false;
#endif
}

usize gpu_dx12_context::CreateCommandQueue(gpu_commandlist_type type, u32 nodeMask, u32 AffinityMask)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = Convert(type);
	queueDesc.NodeMask = nodeMask;

	usize idx = m_impl->commandQueues.insert();
	if (FAILED(m_impl->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_impl->commandQueues[idx]) CFC_DX12_MGPU_ADDAFFINITYMASK)))
	{
		m_impl->commandQueues.erase(idx);
		return invalid_index;
	}
	return idx;

}

usize gpu_dx12_context::CreateCommandAllocator(gpu_commandlist_type type, u32 AffinityMask)
{
	usize idx = m_impl->commandAllocators.insert();
	if (FAILED(m_impl->device->CreateCommandAllocator(Convert(type), IID_PPV_ARGS(&m_impl->commandAllocators[idx]) CFC_DX12_MGPU_ADDAFFINITYMASK )))
	{
		m_impl->commandAllocators.erase(idx);
		return invalid_index;
	}
	return idx;
}

usize gpu_dx12_context::CreateCommandList(gpu_commandlist_type type, usize cmdAllocatorIdx, bool startClosed /*= false*/, usize initialPipelineStateIdx/*=invalid_index*/, u32 nodeMask/*=0*/, u32 AffinityMask)
{
	usize idx = m_impl->commandLists.insert();
	if (FAILED(m_impl->device->CreateCommandList(
		nodeMask,
		Convert(type),
		m_impl->commandAllocators[cmdAllocatorIdx].Get(),
		initialPipelineStateIdx == invalid_index?nullptr:m_impl->pipelineStates[initialPipelineStateIdx].Get(),
		IID_PPV_ARGS(&m_impl->commandLists[idx])
		CFC_DX12_MGPU_ADDAFFINITYMASK)))
	{
		m_impl->commandLists.erase(idx);
		return invalid_index;
	}

	if (startClosed)
		((ID3D12ProxyGraphicsCommandList*)m_impl->commandLists[idx].Get())->Close();
	return idx;
}

usize gpu_dx12_context::CreateFence(u64 initialValue, gpu_fenceshare_type type, u32 AffinityMask)
{
	D3D12_FENCE_FLAGS flags;
	switch (type)
	{
		case gpu_fenceshare_type::Unshared:			flags = D3D12_FENCE_FLAG_NONE; break; 
		case gpu_fenceshare_type::Shared:				flags = D3D12_FENCE_FLAG_SHARED; break;
		case gpu_fenceshare_type::SharedAcrossNodes:	flags = D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER; break;
	}

	usize idx = m_impl->fences.insert();
	if (FAILED(m_impl->device->CreateFence(initialValue, flags, IID_PPV_ARGS(&m_impl->fences[idx]) CFC_DX12_MGPU_ADDAFFINITYMASK)))
	{
		m_impl->fences.erase(idx);
		return invalid_index;
	}
	return idx;

}

usize gpu_dx12_context::CreateFenceEvent()
{
	usize idx = m_impl->fenceEvents.insert();
	m_impl->fenceEvents[idx] = std::move(std::make_unique<_imp_dx12_fence_event>());
	return idx;
}

usize gpu_dx12_context::CreateDescriptorHeap(gpu_descriptorheap_type type, u32 numDescriptors, bool shaderVisible, u32 nodeMask/*=0*/, u32 AffinityMask)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Type = Convert(type);
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = numDescriptors;
	desc.NodeMask = nodeMask;

	usize idx = m_impl->descriptorHeaps.insert();
	if (FAILED(m_impl->device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_impl->descriptorHeaps[idx]) CFC_DX12_MGPU_ADDAFFINITYMASK)))
	{
		m_impl->descriptorHeaps.erase(idx);
		return invalid_index;
	}
	return idx;
}

usize gpu_dx12_context::CreateQueryHeap(gpu_queryheap_type type, u32 numQueries, u32 nodeMask /*=0*/, u32 AffinityMask)
{
	D3D12_QUERY_HEAP_DESC desc;
	desc.Type = Convert(type);
	desc.Count = numQueries;
	desc.NodeMask = nodeMask;

	usize idx = m_impl->queryHeaps.insert();
	if (FAILED(m_impl->device->CreateQueryHeap(&desc, IID_PPV_ARGS(&m_impl->queryHeaps[idx]) CFC_DX12_MGPU_ADDAFFINITYMASK)))
	{
		m_impl->queryHeaps.erase(idx);
		return invalid_index;
	}
	return idx;
}

usize gpu_dx12_context::CreateSwapChain(usize cmdQueueIdx, i32 width, i32 height, void* windowHandle, bool windowed /*= true*/, gpu_swapimage_type imgType /*= gpu_swapimage_type::RGBA8UN*/, gpu_swapflip_type flipType /*= gpu_swapflip_type::Discard*/, i32 numBuffers /*= 2*/, i32 multisampleCount /*= 1*/, i32 multisampleQuality/*=0*/)
{
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.BufferCount = numBuffers;
	desc.Width = width;
	desc.Height = height;
	switch (imgType)
	{
		case gpu_swapimage_type::Rgba8Unorm:			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
		case gpu_swapimage_type::Rgba8UnormSrgb:		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
		case gpu_swapimage_type::Rgb10A2Unorm:			desc.Format = DXGI_FORMAT_R10G10B10A2_UNORM; break;
		case gpu_swapimage_type::Rgba16FloatSrgb:		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
	}
	
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SwapEffect = (flipType == gpu_swapflip_type::Discard )? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.SampleDesc.Count = multisampleCount;
	desc.SampleDesc.Quality = multisampleQuality;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	usize idx = m_impl->swapChains.insert();
	ComPtr<IDXGISwapChain1> swapchain;

#ifdef CFC_DX12_MGPU_AFFINITY
	if (FAILED(m_impl->factory->CreateSwapChainForHwnd(m_impl->commandQueues[cmdQueueIdx]->GetChildObject(0), (HWND)windowHandle, &desc, nullptr, nullptr, &swapchain)))
#else
	if (FAILED(m_impl->factory->CreateSwapChainForHwnd(m_impl->commandQueues[cmdQueueIdx].Get(), (HWND)windowHandle, &desc,	nullptr, nullptr, &swapchain)))
#endif
	{
		m_impl->swapChains.erase(idx);
		return invalid_index;
	}
#ifdef CFC_DX12_MGPU_AFFINITY
	if (FAILED(DXGIXAffinityCreateLDASwapChain(swapchain.Get(), m_impl->commandQueues[cmdQueueIdx].Get(), m_impl->device.Get(), m_impl->swapChains[idx].ReleaseAndGetAddressOf())))
	{
		m_impl->swapChains.erase(idx);
		return invalid_index;
	}
#else
	swapchain.As(&m_impl->swapChains[idx]);
#endif
	return idx;
}

usize gpu_dx12_context::CreateRootSignature(const gpu_rootsignature_desc& desc, u32 AffinityMask)
{
	D3D12_ROOT_SIGNATURE_DESC rdesc;
	rdesc.Flags = (D3D12_ROOT_SIGNATURE_FLAGS)desc.Flags;
	rdesc.NumParameters = (u32)desc.Parameters.size();
	rdesc.NumStaticSamplers = (u32)desc.StaticSamplers.size();
	rdesc.pParameters = rdesc.NumParameters==0 ?nullptr: (D3D12_ROOT_PARAMETER*)_alloca(sizeof(D3D12_ROOT_PARAMETER) * rdesc.NumParameters);
	rdesc.pStaticSamplers = rdesc.NumStaticSamplers == 0 ? nullptr : (D3D12_STATIC_SAMPLER_DESC*) _alloca(sizeof(D3D12_STATIC_SAMPLER_DESC) * rdesc.NumStaticSamplers);

	// count needed memory for descriptor table ranges
	usize numRanges = 0;
	for (usize i = 0; i < desc.Parameters.size(); i++)
	{
		auto& param = desc.Parameters[i];
		if (param.Type == gpu_rootsignature_desc::RptDescriptorTable)
			numRanges += param.DescriptorTable_Ranges.size();
	}

	// make ranges
	D3D12_DESCRIPTOR_RANGE* ranges = (D3D12_DESCRIPTOR_RANGE*)_alloca(sizeof(D3D12_DESCRIPTOR_RANGE) * numRanges);

	// populate parameters
	for (usize i = 0; i < desc.Parameters.size(); i++)
	{
		auto& param = desc.Parameters[i];
		auto& rparam = (D3D12_ROOT_PARAMETER&)rdesc.pParameters[i];
		rparam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)(param.Type);
		rparam.ShaderVisibility = Convert(param.ShaderVisibility);
		switch (param.Type)
		{
		case gpu_rootsignature_desc::Rpt32BitCsts:
			rparam.Constants.Num32BitValues = param.CstsNum32BitValues;
			rparam.Constants.RegisterSpace = param.CstsDescRegisterSpace;
			rparam.Constants.ShaderRegister = param.CstsDescShaderRegister;
			break;
		case gpu_rootsignature_desc::RptDescriptorTable:
			rparam.DescriptorTable.NumDescriptorRanges = (u32)param.DescriptorTable_Ranges.size();
			rparam.DescriptorTable.pDescriptorRanges = rparam.DescriptorTable.NumDescriptorRanges == 0? nullptr:ranges;
			ranges += rparam.DescriptorTable.NumDescriptorRanges;
			for (u32 j = 0; j < rparam.DescriptorTable.NumDescriptorRanges; j++)
			{
				auto& range = param.DescriptorTable_Ranges[j];
				auto& rrange = (D3D12_DESCRIPTOR_RANGE&)rparam.DescriptorTable.pDescriptorRanges[j];
				rrange.RangeType = Convert(range.Type);
				rrange.BaseShaderRegister = range.BaseShaderRegister;
				rrange.NumDescriptors = range.NumDescriptors;
				rrange.OffsetInDescriptorsFromTableStart = range.OffsetInDescriptorsFromTableStart;
				rrange.RegisterSpace = range.RegisterSpace;
			}
			break;
		case gpu_rootsignature_desc::RptCbv:
		case gpu_rootsignature_desc::RptSrv:
		case gpu_rootsignature_desc::RptUav:
			rparam.Descriptor.RegisterSpace = param.CstsDescRegisterSpace;
			rparam.Descriptor.ShaderRegister = param.CstsDescShaderRegister;
			break;
		}
	}

	// populate static samplers
	for (usize i = 0; i < desc.StaticSamplers.size(); i++)
	{
		auto& sampler = desc.StaticSamplers[i];
		auto& rsampler = (D3D12_STATIC_SAMPLER_DESC&)rdesc.pStaticSamplers[i];
		rsampler.Filter = Convert(sampler.Filter);
		rsampler.AddressU = Convert(sampler.AddressU);
		rsampler.AddressV = Convert(sampler.AddressV);
		rsampler.AddressW = Convert(sampler.AddressW);
		rsampler.MipLODBias = sampler.MipLODBias;
		rsampler.MaxAnisotropy = sampler.MaxAnisotropy;
		rsampler.ComparisonFunc = Convert(sampler.ComparisonFunc);
		rsampler.BorderColor = (D3D12_STATIC_BORDER_COLOR)sampler.BorderColor;
		rsampler.MinLOD = sampler.MinLOD;
		rsampler.MaxLOD = sampler.MaxLOD;
		rsampler.ShaderRegister = sampler.ShaderRegister;
		rsampler.RegisterSpace = sampler.RegisterSpace;
		rsampler.ShaderVisibility = Convert(sampler.ShaderVisibility);
	}

	ComPtr<ID3D12ProxyRootSignature> rootSignature;
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	if (FAILED(D3D12SerializeRootSignature(&rdesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)))
	{
		printf("Root signature error:\n %s\n", (char*)error->GetBufferPointer());
		return invalid_index;
	}
	if(FAILED(m_impl->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)) CFC_DX12_MGPU_ADDAFFINITYMASK))
		return invalid_index;

	// root signature is valid, add.
	usize idx=m_impl->rootSignatures.insert();
	m_impl->rootSignatures[idx] = rootSignature;
	return idx;
}

usize gpu_dx12_context::CreatePipelineState(const gpu_graphicspipelinestate_desc& gpsDesc, u32 AffinityMask)
{
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[32];
	usize num = gpsDesc.NumInputElements();
	for (usize i = 0; i < num; i++)
		inputElementDescs[i] = Convert(gpsDesc.InputElements[i]);

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_PROXY_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs,  (u32)gpsDesc.NumInputElements() };
	psoDesc.pRootSignature = m_impl->rootSignatures[gpsDesc.RootSignatureIdx].Get();	// root signature is mandatory
	if(gpsDesc.VertexShaderIdx != invalid_index) 
		psoDesc.VS = { reinterpret_cast<UINT8*>(m_impl->shaderBlobs[gpsDesc.VertexShaderIdx].blob->GetBufferPointer()), m_impl->shaderBlobs[gpsDesc.VertexShaderIdx].blob->GetBufferSize() };
	if (gpsDesc.PixelShaderIdx != invalid_index)
		psoDesc.PS = { reinterpret_cast<UINT8*>(m_impl->shaderBlobs[gpsDesc.PixelShaderIdx].blob->GetBufferPointer()), m_impl->shaderBlobs[gpsDesc.PixelShaderIdx].blob->GetBufferSize() };
	if (gpsDesc.GeometryShaderIdx != invalid_index)
		psoDesc.GS = { reinterpret_cast<UINT8*>(m_impl->shaderBlobs[gpsDesc.GeometryShaderIdx].blob->GetBufferPointer()), m_impl->shaderBlobs[gpsDesc.GeometryShaderIdx].blob->GetBufferSize() };
	if (gpsDesc.DomainShaderIdx != invalid_index)
		psoDesc.DS = { reinterpret_cast<UINT8*>(m_impl->shaderBlobs[gpsDesc.DomainShaderIdx].blob->GetBufferPointer()), m_impl->shaderBlobs[gpsDesc.DomainShaderIdx].blob->GetBufferSize() };
	if (gpsDesc.HullShaderIdx != invalid_index)
		psoDesc.HS = { reinterpret_cast<UINT8*>(m_impl->shaderBlobs[gpsDesc.HullShaderIdx].blob->GetBufferPointer()), m_impl->shaderBlobs[gpsDesc.HullShaderIdx].blob->GetBufferSize() };
	psoDesc.RasterizerState = Convert(gpsDesc.RasterizerState);
	psoDesc.BlendState = Convert(gpsDesc.BlendState);
	psoDesc.DepthStencilState = Convert(gpsDesc.DepthStencilState);
	psoDesc.SampleMask = gpsDesc.SampleMask;
	psoDesc.PrimitiveTopologyType = Convert(gpsDesc.PrimitiveTopology);
	psoDesc.NumRenderTargets = gpsDesc.NumRenderTargets;
	for (int i = 0; i < 8; i++)
		psoDesc.RTVFormats[i] = Convert(gpsDesc.RTVFormats[i]);
	psoDesc.DSVFormat = Convert(gpsDesc.DSVFormat);
	psoDesc.SampleDesc.Count = gpsDesc.MultisampleCount;
	psoDesc.SampleDesc.Quality = gpsDesc.MultisampleQuality;
	psoDesc.IBStripCutValue = Convert(gpsDesc.IBStripCutValue);

	ComPtr<ID3D12ProxyPipelineState> pipelineState;
	if (FAILED(m_impl->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)) CFC_DX12_MGPU_ADDAFFINITYMASK))
		return invalid_index;

	usize idx = m_impl->pipelineStates.insert();
	m_impl->pipelineStates[idx] = pipelineState;
	return idx;
}

usize gpu_dx12_context::CreatePipelineState(const gpu_computepipelinestate_desc& desc, u32 AffinityMask)
{
	// Describe and create the compute pipeline state object (PSO).
	D3D12_PROXY_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_impl->rootSignatures[desc.RootSignatureIdx].Get();	// root signature is mandatory
	if (desc.ComputeShaderIdx != invalid_index)
		psoDesc.CS = { reinterpret_cast<UINT8*>(m_impl->shaderBlobs[desc.ComputeShaderIdx].blob->GetBufferPointer()), m_impl->shaderBlobs[desc.ComputeShaderIdx].blob->GetBufferSize() };
	psoDesc.NodeMask = desc.NodeMask;

	ComPtr<ID3D12ProxyPipelineState> pipelineState;
	if (FAILED(m_impl->device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState) CFC_DX12_MGPU_ADDAFFINITYMASK)))
		return invalid_index;

	usize idx = m_impl->pipelineStates.insert();
	m_impl->pipelineStates[idx] = pipelineState;
	return idx;
}

usize gpu_dx12_context::CreateCommittedResource(gpu_heap_type type, const gpu_resource_desc & resDesc, gpu_resourcestate::flag currentState, const gpu_defaultclear_desc * pDefaultClear, u32 flags, gpu_pageproperty_type pageproperties)
{
	/*
	When a resource is created together with a D3D12_HEAP_TYPE_UPLOAD heap, InitialResourceState must be D3D12_RESOURCE_STATE_GENERIC_READ. 
	When a resource is created together with a D3D12_HEAP_TYPE_READBACK heap, InitialResourceState must be D3D12_RESOURCE_STATE_COPY_DEST.
	https://msdn.microsoft.com/en-us/library/windows/desktop/dn899178(v=vs.85).aspx
	*/
	if (type == gpu_heap_type::Upload)
		stl_assert(currentState == gpu_resourcestate::GenericRead)
	else if (type == gpu_heap_type::Readback)
		stl_assert(currentState == gpu_resourcestate::CopyDestination)

	CD3DX12_HEAP_PROPERTIES heapProps(Convert(type));
	heapProps.CPUPageProperty = (D3D12_CPU_PAGE_PROPERTY)pageproperties;
	auto rdesc = Convert(resDesc);

	D3D12_CLEAR_VALUE rclear;
	if (pDefaultClear)
		rclear = Convert(*pDefaultClear);

	ComPtr<ID3D12ProxyResource> resource;
	HRESULT hr=m_impl->device->CreateCommittedResource(&heapProps, (D3D12_HEAP_FLAGS)flags, &rdesc, Convert(currentState), pDefaultClear?&rclear:nullptr, IID_PPV_ARGS(&resource));
	if (FAILED(hr))
		return invalid_index;

	usize idx = m_impl->resources.insert();
	m_impl->resources[idx].heapType = type;
	m_impl->resources[idx].desc = resDesc;
	m_impl->resources[idx].resource = resource;
	if(resDesc.Dimension == gpu_resource_desc::dimension::Buffer)
		m_impl->resources[idx].gpuVirtualAddress = m_impl->resources[idx].resource->GetGPUVirtualAddress();
	else
		m_impl->resources[idx].gpuVirtualAddress = 0;
	return idx;
}




usize gpu_dx12_context::CreateSwapChainResource(usize swapChainIdx, u32 bufferID)
{
	ComPtr<ID3D12ProxyResource> resource;
	HRESULT hr=m_impl->swapChains[swapChainIdx]->GetBuffer(bufferID, IID_PPV_ARGS(&resource));
	if (FAILED(hr))
		return invalid_index;

	usize idx = m_impl->resources.insert();
	m_impl->resources[idx].heapType = gpu_heap_type::Custom;
	m_impl->resources[idx].resource = resource;
	m_impl->resources[idx].gpuVirtualAddress = 0;
	return idx;
}

bool gpu_dx12_context::CreateDescriptorRTVTexture(usize resourceRenderTargetIdx, usize cpuDescriptorOffset, gpu_format_type fmt /* = gpu_format_type::Unknown*/, u32 mipSlice /*= 0*/, u32 planeSlice /*= 0*/, u32 firstArraySlice /*= 0*/, u32 arraySize /*= ~0*/)
{
	auto& resourceData = m_impl->resources[resourceRenderTargetIdx];
	auto resourceDesc = resourceData.resource->GetDesc();

	D3D12_RENDER_TARGET_VIEW_DESC desc;
	desc.Texture2DArray.MipSlice = mipSlice;
	desc.Texture2DArray.FirstArraySlice = firstArraySlice;
	desc.Texture2DArray.ArraySize = (arraySize == (~0)) ? resourceDesc.DepthOrArraySize - firstArraySlice : arraySize;
	desc.Texture2DArray.PlaneSlice = planeSlice;

	switch (resourceDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_UNKNOWN:
		desc.ViewDimension = D3D12_RTV_DIMENSION_UNKNOWN; break;
	case D3D12_RESOURCE_DIMENSION_BUFFER:
		return false;
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		if (resourceDesc.DepthOrArraySize > 1)
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
		else
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (resourceDesc.SampleDesc.Count > 1)
		{
			if (resourceDesc.DepthOrArraySize > 1)
			{
				// different offsets
				desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
				desc.Texture2DMSArray.ArraySize = (arraySize == (~0)) ? resourceDesc.DepthOrArraySize-firstArraySlice : arraySize;
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
			}
			else
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			if (resourceDesc.DepthOrArraySize > 1)
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			else
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D; break;
	}
	desc.Format = Convert(fmt);

	D3D12_CPU_DESCRIPTOR_HANDLE hdl;
	hdl.ptr = cpuDescriptorOffset;
	m_impl->device->CreateRenderTargetView(resourceData.resource.Get(), &desc, hdl);
	return true;
}

bool gpu_dx12_context::CreateDescriptorDSVTexture(usize resourceDepthStencilIdx, usize cpuDescriptorOffset, bool readOnlyDepth/*=false*/, bool readOnlyStencil /*= false*/, gpu_format_type fmt /*= gpu_format_type::Unknown*/, u32 mipSlice /*= 0*/, u32 planeSlice /*= 0*/, u32 firstArraySlice /*= 0*/, u32 arraySize /*= ~0*/)
{
	auto& resource = m_impl->resources[resourceDepthStencilIdx].resource;
	auto resourceDesc = resource->GetDesc();

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;
	desc.Texture2DArray.MipSlice = mipSlice;
	desc.Texture2DArray.FirstArraySlice = firstArraySlice;
	desc.Texture2DArray.ArraySize = (arraySize == (~0)) ? resourceDesc.DepthOrArraySize-firstArraySlice : arraySize;

	switch (resourceDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_UNKNOWN:
		desc.ViewDimension = D3D12_DSV_DIMENSION_UNKNOWN; break;
	case D3D12_RESOURCE_DIMENSION_BUFFER:
		return false;
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		if (resourceDesc.DepthOrArraySize > 1)
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
		else
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (resourceDesc.SampleDesc.Count > 1)
		{
			if (resourceDesc.DepthOrArraySize > 1)
			{
				// different offsets
				desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
				desc.Texture2DMSArray.ArraySize = (arraySize == (~0)) ? resourceDesc.DepthOrArraySize-firstArraySlice : arraySize;
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
			}
			else
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			if (resourceDesc.DepthOrArraySize > 1)
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			else
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		return false;
	}
	desc.Format = Convert(fmt);
	desc.Flags = (D3D12_DSV_FLAGS)((readOnlyDepth ? D3D12_DSV_FLAG_READ_ONLY_DEPTH : 0) | (readOnlyStencil ? D3D12_DSV_FLAG_READ_ONLY_STENCIL : 0));

	D3D12_CPU_DESCRIPTOR_HANDLE hdl;
	hdl.ptr = cpuDescriptorOffset;
	m_impl->device->CreateDepthStencilView(resource.Get(), &desc, hdl);
	return true;
}

bool cfc::gpu_dx12_context::CreateDescriptorSRVTexture(usize resourceTextureIdx, usize cpuDescriptorOffset, gpu_format_type fmt /*= gpu_format_type::Unknown*/, u32 mostDetailedMip/*=0*/, u32 mipLevels /*= ~0*/, f32 resourceMinLodClamp /*= 0.0f*/, u32 planeSlice /*= 0*/, u32 firstArraySlice /*= 0*/, u32 arraySize /*= ~0*/)
{
	Microsoft::WRL::ComPtr<ID3D12ProxyResource> resNull;
	auto& resource = (resourceTextureIdx != cfc::invalid_index?m_impl->resources[resourceTextureIdx].resource:resNull);
	D3D12_RESOURCE_DESC resourceDesc;
	if(resource != nullptr)
		resourceDesc = resource->GetDesc();
	else
		memset(&resourceDesc, 0, sizeof(D3D12_RESOURCE_DESC));

	D3D12_SHADER_RESOURCE_VIEW_DESC desc;

	switch (resourceDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_UNKNOWN:
		return false;
	case D3D12_RESOURCE_DIMENSION_BUFFER:
		return false;
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		if (resourceDesc.DepthOrArraySize > 1)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MipLevels = mipLevels;
			desc.Texture1DArray.MostDetailedMip = mostDetailedMip;
			desc.Texture1DArray.ResourceMinLODClamp = resourceMinLodClamp;
			desc.Texture1DArray.FirstArraySlice = firstArraySlice;
			desc.Texture1DArray.ArraySize = (arraySize==(~0))?resourceDesc.DepthOrArraySize-firstArraySlice:arraySize;
		}
		else
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipLevels = mipLevels;
			desc.Texture1D.MostDetailedMip = mostDetailedMip;
			desc.Texture1D.ResourceMinLODClamp = resourceMinLodClamp;
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (resourceDesc.SampleDesc.Count > 1)
		{
			if (resourceDesc.DepthOrArraySize > 1)
			{
				desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
				desc.Texture2DMSArray.ArraySize = (arraySize == (~0)) ? resourceDesc.DepthOrArraySize - firstArraySlice : arraySize;
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
			}
			else
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			if (resourceDesc.DepthOrArraySize > 1)
			{
				desc.Texture2DArray.MipLevels = mipLevels;
				desc.Texture2DArray.MostDetailedMip = mostDetailedMip;
				desc.Texture2DArray.ResourceMinLODClamp = resourceMinLodClamp;
				desc.Texture2DArray.FirstArraySlice = firstArraySlice;
				desc.Texture2DArray.ArraySize = (arraySize == (~0)) ? resourceDesc.DepthOrArraySize - firstArraySlice : arraySize;
				desc.Texture2DArray.PlaneSlice = planeSlice;
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			}
			else
			{
				desc.Texture2D.MipLevels = mipLevels;
				desc.Texture2D.MostDetailedMip = mostDetailedMip;
				desc.Texture2D.ResourceMinLODClamp = resourceMinLodClamp;
				desc.Texture2D.PlaneSlice = planeSlice;
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			}
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		desc.Texture3D.MipLevels = mipLevels;
		desc.Texture3D.MostDetailedMip = mostDetailedMip;
		desc.Texture3D.ResourceMinLODClamp = resourceMinLodClamp;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		break;
	}
	desc.Format = Convert(fmt);
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	D3D12_CPU_DESCRIPTOR_HANDLE hdl;
	hdl.ptr = cpuDescriptorOffset;
	m_impl->device->CreateShaderResourceView(resource.Get(), &desc, hdl);
	return true;
}

bool cfc::gpu_dx12_context::CreateDescriptorUAVTexture(usize resourceTextureIdx, usize cpuDescriptorOffset, gpu_format_type fmt /*= gpu_format_type::Unknown*/, u32 mipSlice /*= 0*/, u32 planeSlice /*= 0*/, u32 firstArraySlice /*= 0*/, u32 arraySize /*= ~0*/)
{
	Microsoft::WRL::ComPtr<ID3D12ProxyResource> resNull;
	auto& resource = (resourceTextureIdx != cfc::invalid_index ? m_impl->resources[resourceTextureIdx].resource : resNull);
	D3D12_RESOURCE_DESC resourceDesc;
	if (resource != nullptr)
		resourceDesc = resource->GetDesc();
	else
		memset(&resourceDesc, 0, sizeof(D3D12_RESOURCE_DESC));

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;

	switch (resourceDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_UNKNOWN:
		return false;
	case D3D12_RESOURCE_DIMENSION_BUFFER:
		return false;
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		if (resourceDesc.DepthOrArraySize > 1)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MipSlice = mipSlice;
			desc.Texture1DArray.FirstArraySlice = firstArraySlice;
			desc.Texture1DArray.ArraySize = (arraySize == (~0)) ? resourceDesc.DepthOrArraySize-firstArraySlice : arraySize;
		}
		else
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = mipSlice;
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (resourceDesc.SampleDesc.Count > 1)
		{
			// multisampling is not supported for RW textures
			stl_assert(false);
		}
		else
		{
			if (resourceDesc.DepthOrArraySize > 1)
			{
				desc.Texture2DArray.FirstArraySlice = firstArraySlice;
				desc.Texture2DArray.MipSlice = mipSlice;
				desc.Texture2DArray.PlaneSlice = planeSlice;
				desc.Texture2DArray.ArraySize = (arraySize == (~0)) ? resourceDesc.DepthOrArraySize - firstArraySlice : arraySize;
				desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			}
			else
			{
				desc.Texture2D.MipSlice = mipSlice;
				desc.Texture2D.PlaneSlice = planeSlice;
				desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			}
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		desc.Texture3D.FirstWSlice = firstArraySlice;
		desc.Texture3D.WSize = arraySize;
		desc.Texture3D.MipSlice = mipSlice;
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
		break;
	}
	desc.Format = Convert(fmt);

	D3D12_CPU_DESCRIPTOR_HANDLE hdl;
	hdl.ptr = cpuDescriptorOffset;
	m_impl->device->CreateUnorderedAccessView(resource.Get(), nullptr, &desc, hdl);
	return true;
}

bool cfc::gpu_dx12_context::CreateDescriptorSRVBuffer(usize resourceBufferIdx, usize cpuDescriptorOffset, u32 structureByteStride/*=0*/, u64 firstElement /*= 0*/, u32 numElements /*= ~0*/)
{
	bool raw = (structureByteStride == 0);
	Microsoft::WRL::ComPtr<ID3D12ProxyResource> resNull;
	auto& resource = (resourceBufferIdx != cfc::invalid_index ? m_impl->resources[resourceBufferIdx].resource : resNull);

	if(numElements == ~0)
	{
		// detect number of elements
		auto rdesc = resource->GetDesc();
		u32 divisor = (structureByteStride == 0 ? 4 : structureByteStride);
		stl_assert((rdesc.Width % divisor) == 0); // check if we can actually fit the objects in here..
		numElements = (u32)(rdesc.Width / divisor);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = raw ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Buffer.FirstElement = firstElement;
	desc.Buffer.NumElements = numElements;
	desc.Buffer.StructureByteStride = raw ? 0 : structureByteStride;
	desc.Buffer.Flags = raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE hdl;
	hdl.ptr = cpuDescriptorOffset;
	m_impl->device->CreateShaderResourceView(resource.Get(), &desc, hdl);
	return true;
}

bool cfc::gpu_dx12_context::CreateDescriptorUAVBuffer(usize resourceBufferIdx, usize cpuDescriptorOffset, u32 structureByteStride/*=0*/, u64 firstElement /*= 0*/, u32 numElements /*= ~0*/, u64 counterOffsetInBytes /*= 0*/, usize counterResourceIdx/*=cfc::invalid_index*/)
{
	Microsoft::WRL::ComPtr<ID3D12ProxyResource> resNull;
	auto& resource = (resourceBufferIdx != cfc::invalid_index ? m_impl->resources[resourceBufferIdx].resource : resNull);
	auto& ctrResource = (counterResourceIdx != cfc::invalid_index ? m_impl->resources[counterResourceIdx].resource : resNull);
	bool raw = (structureByteStride == 0);

	if (numElements == ~0)
	{
		// detect number of elements
		auto rdesc = resource->GetDesc();
		u32 divisor = (structureByteStride == 0 ? 1 : structureByteStride);
		stl_assert((rdesc.Width % divisor) == 0); // check if we can actually fit the objects in here..
		numElements = (u32)(rdesc.Width / divisor);
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = raw ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = firstElement;
	desc.Buffer.NumElements = numElements;
	desc.Buffer.CounterOffsetInBytes = counterOffsetInBytes;
	desc.Buffer.StructureByteStride = raw ? 0 : structureByteStride;
	desc.Buffer.Flags = raw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;

	ID3D12ProxyResource* counterBuffer = counterOffsetInBytes == 0 ? nullptr : ctrResource.Get();

	D3D12_CPU_DESCRIPTOR_HANDLE hdl;
	hdl.ptr = cpuDescriptorOffset;
	m_impl->device->CreateUnorderedAccessView(resource.Get(), counterBuffer, &desc, hdl);
	return true;
}

bool gpu_dx12_context::CreateDescriptorCBVBuffer(usize resourceBufferIdx, usize cpuDescriptorOffset, u64 offset)
{
	Microsoft::WRL::ComPtr<ID3D12ProxyResource> resNull;
	auto& resource = (resourceBufferIdx != cfc::invalid_index ? m_impl->resources[resourceBufferIdx].resource : resNull);
	auto rdesc = resource->GetDesc();

	stl_assert(offset % 256 == 0); // dx12 mandatory constant buffer offset alignment
	stl_assert(offset < rdesc.Width); // check if out of bounds

	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
	desc.BufferLocation = ResourceGetGPUAddress(resourceBufferIdx) + offset;
	desc.SizeInBytes = (u32)(stl_math_iroundup(rdesc.Width - offset, 256));

	D3D12_CPU_DESCRIPTOR_HANDLE hdl;
	hdl.ptr = cpuDescriptorOffset;
	m_impl->device->CreateConstantBufferView(&desc, hdl);
	return true;
}

bool cfc::gpu_dx12_context::CreateDescriptorSampler(usize cpuDescriptorOffset, const cfc::gpu_sampler_desc& smpDesc)
{
	D3D12_CPU_DESCRIPTOR_HANDLE hdl;
	hdl.ptr = cpuDescriptorOffset;
	m_impl->device->CreateSampler(&Convert(smpDesc), hdl);
	return true;
}

usize gpu_dx12_context::CreateShaderBlobCompile(const void* sourceData, usize sourceDataLength, const char* entryPoint, const char* shaderTarget, const char* sourceName /*= ""*/, stl_string* outError/*=nullptr*/)
{
	u32 flags = 0;
#if defined(CFC_DX12_ENABLE_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_PARTIAL_PRECISION;
#endif
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> shaderError;
	if (FAILED(D3DCompile(sourceData, sourceDataLength, sourceName, nullptr, nullptr, entryPoint, shaderTarget, flags, 0, &shaderBlob, &shaderError)))
	{
		// report error
		if (outError)
			*outError = stl_string((char*)shaderError->GetBufferPointer(), shaderError->GetBufferSize());

		return invalid_index;
	}

	// root signature is valid, add.
	
	usize idx = m_impl->shaderBlobs.insert();
	m_impl->shaderBlobs[idx].blob = shaderBlob;
	ShaderBlobReflect(idx, m_impl->shaderBlobs[idx].reflected);
	return idx;
}

const gpu_shaderreflection_desc& cfc::gpu_dx12_context::ShaderBlobGetReflection(usize shaderIdx)
{
	return m_impl->shaderBlobs[shaderIdx].reflected;
}

void gpu_dx12_context::MGPUSwitchToNextNode()
{
#ifdef CFC_DX12_MGPU_AFFINITY
	m_impl->device->SwitchToNextNode();
#endif
}

u32 gpu_dx12_context::MGPUGetCurrentNodeID()
{
#ifdef CFC_DX12_MGPU_AFFINITY
	return m_impl->device->GetActiveNodeIndex();
#endif
	return 0;
}

u32 gpu_dx12_context::MGPUGetAllNodeMask()
{
#ifdef CFC_DX12_MGPU_AFFINITY
	return m_impl->device->GetNodeMask();
#endif
	return 0;
}

void gpu_dx12_context::Destroy(gpu_object_type type, usize idx)
{
	switch (type)
	{
		case gpu_object_type::CommandQueue:			m_impl->commandQueues.erase(idx); break;
		case gpu_object_type::CommandAllocator:		m_impl->commandAllocators.erase(idx); break;
		case gpu_object_type::CommandList:			m_impl->commandLists.erase(idx); break;
		case gpu_object_type::Fence:				m_impl->fences.erase(idx); break;
		case gpu_object_type::FenceEvent:			m_impl->fenceEvents.erase(idx); break;
		case gpu_object_type::DescriptorHeap:		m_impl->descriptorHeaps.erase(idx); break;
		case gpu_object_type::QueryHeap:			m_impl->queryHeaps.erase(idx); break;
		case gpu_object_type::SwapChain:			m_impl->swapChains.erase(idx); break;
		case gpu_object_type::ShaderBlob:			m_impl->shaderBlobs.erase(idx); break;
		case gpu_object_type::RootSignature:		m_impl->rootSignatures.erase(idx); break;
		case gpu_object_type::PipelineState:		m_impl->pipelineStates.erase(idx); break;
		case gpu_object_type::Resource:				m_impl->resources.erase(idx); break;
		default:								stl_assert(false); break; // unimplemented
	}
}

void* gpu_dx12_context::ResourceMap(usize resourceIdx, bool read, u32 subResource, usize startByteOffset /* = 0 */, usize endByteOffset /* = 0 */)
{
	ID3D12ProxyResource* res = m_impl->resources[resourceIdx].resource.Get();
	void* ret = nullptr;
	CD3DX12_RANGE range(startByteOffset, endByteOffset);
	HRESULT result = res->Map(subResource, &range, &ret);

	stl_assert(result == S_OK);

	return ret;
}

void gpu_dx12_context::ResourceUnmap(usize resourceIdx, bool write, u32 subResource, usize startByteOffset /* = 0 */, usize endByteOffset /* = 0 */)
{
	ID3D12ProxyResource* res = m_impl->resources[resourceIdx].resource.Get();
	CD3DX12_RANGE range(startByteOffset, endByteOffset);
	res->Unmap(subResource, &range);
}

bool gpu_dx12_context::CommandAllocatorReset(usize commandAllocatorIdx)
{
	return SUCCEEDED(m_impl->commandAllocators[commandAllocatorIdx]->Reset());
}

bool gpu_dx12_context::CommandQueueSignal(usize commandQueueIdx, usize fenceIdx, u64 fenceValue)
{
	return SUCCEEDED(m_impl->commandQueues[commandQueueIdx]->Signal(m_impl->fences[fenceIdx].Get(), fenceValue));
}

bool gpu_dx12_context::CommandQueueWait(usize CommandQueueIdx, usize fenceIdx, u64 fenceValue)
{
	return SUCCEEDED(m_impl->commandQueues[CommandQueueIdx]->Wait(m_impl->fences[fenceIdx].Get(), fenceValue));
}

u32 gpu_dx12_context::SwapchainGetCurrentBackbufferIndex(usize swapChainIdx)
{	
#ifndef CFC_DX12_MGPU_AFFINITY
	return reinterpret_cast<IDXGISwapChain3*>(m_impl->swapChains[swapChainIdx].Get())->GetCurrentBackBufferIndex();
#else
	return m_impl->swapChains[swapChainIdx]->GetCurrentBackBufferIndex();
#endif
}

bool cfc::gpu_dx12_context::FenceSetName(usize fenceID, const i8* fenceName)
{
	stl_string str = stl_string_advanced::utf16_fromUtf8(fenceName); str.push_back(0);
	return SUCCEEDED(m_impl->fences[fenceID]->SetName((LPCWSTR)str.c_str()));
}

bool gpu_dx12_context::FenceSetEventOnCompletion(usize fenceID, usize fenceEventID, u64 fenceValue)
{
	return SUCCEEDED(m_impl->fences[fenceID]->SetEventOnCompletion(fenceValue, m_impl->fenceEvents[fenceEventID]->ev));
}

u64 gpu_dx12_context::FenceGetCompletedValue(usize fenceID)
{
	return m_impl->fences[fenceID]->GetCompletedValue();
}

bool gpu_dx12_context::SwapchainPresent(usize swapChainIdx, u32 swapInterval, u32 flags)
{
	return SUCCEEDED(m_impl->swapChains[swapChainIdx]->Present(swapInterval, flags));
}

bool gpu_dx12_context::CommandQueueExecuteCommandLists(usize commandQueueIdx, u32 numCommandLists, const usize* cmdListIdxArray)
{
	ID3D12ProxyCommandList** commandLists = (ID3D12ProxyCommandList**)_alloca(numCommandLists*sizeof(ID3D12CommandList*));
	for (u32 i = 0; i < numCommandLists; i++)
		commandLists[i] = m_impl->commandLists[cmdListIdxArray[i]].Get();
	m_impl->commandQueues[commandQueueIdx]->ExecuteCommandLists(numCommandLists, commandLists);
	return true;
}

bool gpu_dx12_context::FenceEventWaitFor(usize fenceEventID, u32 timeout /*= ~0*/)
{
	return WaitForSingleObjectEx(m_impl->fenceEvents[fenceEventID]->ev, timeout, FALSE) == 0;
}

bool gpu_dx12_context::SwapchainResizeBuffer(usize swapChainIdx, u32 width, u32 height, u32 frameCount)
{
	auto& swapChain = m_impl->swapChains[swapChainIdx];
	DXGI_SWAP_CHAIN_DESC desc = {};
	swapChain->GetDesc(&desc);
	return SUCCEEDED(swapChain->ResizeBuffers(frameCount, width, height, desc.BufferDesc.Format, desc.Flags));
}


bool cfc::gpu_dx12_context::SwapchainResizeTarget(usize swapChainIdx, cfc::gpu_format_type fmt, u32 width, u32 height, u32 fpsNumerator, u32 fpsDenominator)
{
	auto& swapChain = m_impl->swapChains[swapChainIdx];
	DXGI_MODE_DESC dsc = {};
	dsc.Format = Convert(fmt);
	dsc.Width = width;
	dsc.Height = height;
	dsc.RefreshRate.Numerator = fpsNumerator;
	dsc.RefreshRate.Denominator = fpsDenominator;
	dsc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dsc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// find closest match
	IDXGIOutput* output = nullptr;
	swapChain->GetContainingOutput(&output);
	DXGI_MODE_DESC dscClosest;
	output->FindClosestMatchingMode(&dsc, &dscClosest, m_impl->device.Get());

	return SUCCEEDED(swapChain->ResizeTarget(&dscClosest));
}


bool cfc::gpu_dx12_context::SwapchainGetFullscreenState(usize swapChainIdx)
{
	auto& swapChain = m_impl->swapChains[swapChainIdx];
	BOOL res;
	swapChain->GetFullscreenState(&res, nullptr);
	return res == TRUE;
}

bool gpu_dx12_context::SwapchainSetFullscreenState(usize swapChainIdx, bool enabled)
{
	auto& swapChain = m_impl->swapChains[swapChainIdx];
	return SUCCEEDED(swapChain->SetFullscreenState((enabled?TRUE:FALSE), nullptr));
}

usize gpu_dx12_context::DescriptorHeapGetCPUAddressStart(usize descriptorHeapIdx)
{
	return m_impl->descriptorHeaps[descriptorHeapIdx]->GetCPUDescriptorHandleForHeapStart().ptr;
}

u64 gpu_dx12_context::DescriptorHeapGetGPUAddressStart(usize descriptorHeapIdx)
{
	return m_impl->descriptorHeaps[descriptorHeapIdx]->GetGPUDescriptorHandleForHeapStart().ptr;
}

bool gpu_dx12_context::ResourceMakeResident(usize* resourceIndices, u32 numResources)
{
	ID3D12ProxyPageable** pagable = (ID3D12ProxyPageable**)_alloca(sizeof(ID3D12ProxyPageable*) * numResources);
	for (u32 i = 0; i < numResources; i++)
		pagable[i] = m_impl->resources[resourceIndices[i]].resource.Get();
	return SUCCEEDED(m_impl->device->MakeResident(numResources, pagable));
}

bool gpu_dx12_context::ResourceEvict(usize* resourceIndices, u32 numResources)
{
	ID3D12ProxyPageable** pagable = (ID3D12ProxyPageable**)_alloca(sizeof(ID3D12ProxyPageable*) * numResources);
	for (u32 i = 0; i < numResources; i++)
		pagable[i] = m_impl->resources[resourceIndices[i]].resource.Get();
	return SUCCEEDED(m_impl->device->Evict(numResources, pagable));
}

u64 gpu_dx12_context::ResourceGetGPUAddress(usize resourceIdx)
{
	return m_impl->resources[resourceIdx].gpuVirtualAddress;
}

const cfc::gpu_resource_desc& cfc::gpu_dx12_context::ResourceGetDesc(usize resourceIdx)
{
	return m_impl->resources[resourceIdx].desc;
}

void cfc::gpu_dx12_context::ResourceSetCustom(usize resourceIdx, void* object)
{
	m_impl->resources[resourceIdx].custom = object;
}

void* cfc::gpu_dx12_context::ResourceGetCustom(usize resourceIdx)
{
	return m_impl->resources[resourceIdx].custom;
}

cfc::gpu_heap_type cfc::gpu_dx12_context::ResourceGetHeapType(usize resourceIdx)
{
	return m_impl->resources[resourceIdx].heapType;
}

bool cfc::gpu_dx12_context::ResourceSetName(usize resourceIdx, const i8* fenceName)
{
	stl_string str = stl_string_advanced::utf16_fromUtf8(fenceName); str.push_back(0);
	return SUCCEEDED(m_impl->resources[resourceIdx].resource->SetName((LPCWSTR)str.c_str()));
}

usize gpu_dx12_context::GetDescriptorIncrementSize(gpu_descriptorheap_type type)
{
	return m_impl->device->GetDescriptorHandleIncrementSize(Convert(type));
}

bool gpu_dx12_context::DeviceIsInited() const
{
	return m_impl->device != nullptr;
}

u32 gpu_dx12_context::DeviceGetNodeCount() const
{
	stl_assert(m_impl->device);
	return m_impl->device->GetNodeCount();
}

bool gpu_dx12_context::ShaderBlobReflect(usize shaderIdx, gpu_shaderreflection_desc& output)
{
	ID3D12ShaderReflection* dxReflector = NULL;
	D3DReflect(m_impl->shaderBlobs[shaderIdx].blob->GetBufferPointer(), m_impl->shaderBlobs[shaderIdx].blob->GetBufferSize(), IID_ID3D12ShaderReflection, (void**)&dxReflector);

	// make variables
	D3D12_SHADER_DESC shrDesc;
	D3D12_SIGNATURE_PARAMETER_DESC pdesc;
	D3D12_SHADER_INPUT_BIND_DESC inputdesc;

	// reflect description
	dxReflector->GetDesc(&shrDesc);
	output.Info.ConstantBuffers = shrDesc.ConstantBuffers;
	output.Info.BoundResources = shrDesc.BoundResources;
	output.Info.InputParameters = shrDesc.InputParameters;   
	output.Info.OutputParameters = shrDesc.OutputParameters; 
		   
	output.Info.InstructionCount = shrDesc.InstructionCount;    
	output.Info.TempRegisterCount = shrDesc.TempRegisterCount;
	output.Info.TempArrayCount = shrDesc.TempArrayCount;
	output.Info.DefCount = shrDesc.DefCount;                   
	output.Info.DclCount = shrDesc.DclCount;

	// reflect thread groups
	dxReflector->GetThreadGroupSize(&output.Info.ComputeThreadGroupX, &output.Info.ComputeThreadGroupY, &output.Info.ComputeThreadGroupZ);

	// reflect inputs
	for (u32 i = 0; i < output.Info.InputParameters; i++)
	{
		gpu_shaderreflection_desc::parameter_desc param;
		dxReflector->GetInputParameterDesc(i, &pdesc);
		param = Convert(pdesc);
		output.Inputs.push_back(param);
	}
	
	// reflect outputs
	for (u32 i = 0; i < output.Info.OutputParameters; i++)
	{
		gpu_shaderreflection_desc::parameter_desc param;
		dxReflector->GetOutputParameterDesc(i, &pdesc);
		param = Convert(pdesc);
		output.Outputs.push_back(param);
	}

	// reflect resources
	for (u32 i = 0; i < output.Info.BoundResources; i++)
	{
		gpu_shaderreflection_desc::gpu_resource_desc resource;
		dxReflector->GetResourceBindingDesc(i, &inputdesc);

		// if resource is a cbuffer, get extra info
		if (inputdesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER)
		{
			gpu_shaderreflection_desc::cbuffer_desc cbufferDesc;
			auto* cbufferData= dxReflector->GetConstantBufferByName(inputdesc.Name);
			D3D12_SHADER_BUFFER_DESC sbdesc;
			cbufferData->GetDesc(&sbdesc);

			cbufferDesc.SizeInBytes = sbdesc.Size;
			cbufferDesc.ResourceIndex = (u32)output.Resources.size();
			for (u32 j = 0; j < sbdesc.Variables; j++)
			{
				auto* cbufferVarData = cbufferData->GetVariableByIndex(j);
				D3D12_SHADER_VARIABLE_DESC svdesc;
				cbufferVarData->GetDesc(&svdesc);

				auto* stype = cbufferVarData->GetType();
				D3D12_SHADER_TYPE_DESC stdesc;
				stype->GetDesc(&stdesc);

				gpu_shaderreflection_desc::cbuffer_var_desc vardesc;
				vardesc.Name = svdesc.Name;
				vardesc.Offset = svdesc.StartOffset;
				vardesc.SizeInBytes = svdesc.Size;

				switch (stdesc.Type)
				{
					case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_VOID: vardesc.Type = gpu_shaderreflection_desc::variable_type::Void;	break;
					case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_DOUBLE: vardesc.Type = gpu_shaderreflection_desc::variable_type::Double;	break;
					case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_FLOAT: vardesc.Type = gpu_shaderreflection_desc::variable_type::Float;	break;
					case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_INT: vardesc.Type = gpu_shaderreflection_desc::variable_type::Int32;	break;
					case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_UINT: vardesc.Type = gpu_shaderreflection_desc::variable_type::Uint32;	break;
					case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_MIN16FLOAT: vardesc.Type = gpu_shaderreflection_desc::variable_type::Half;	break;
					case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_MIN16INT: vardesc.Type = gpu_shaderreflection_desc::variable_type::Int16;	break;
					case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_MIN16UINT: vardesc.Type = gpu_shaderreflection_desc::variable_type::Uint16;	break;
				}

				vardesc.TypeElements = stdesc.Elements;
				vardesc.TypeRows = stdesc.Rows;
				vardesc.TypeColumns = stdesc.Columns;
				cbufferDesc.Variables.push_back(vardesc);
			}
			output.Cbuffers.push_back(cbufferDesc);
			resource.TypeIndex = (u32)output.Cbuffers.size() - 1;
		}

		resource.Name = inputdesc.Name;
		resource.Type = (gpu_shaderreflection_desc::resource_type)inputdesc.Type;
		resource.BindPoint = inputdesc.BindPoint;
		resource.BindCount = inputdesc.BindCount;

		resource.uFlags = inputdesc.uFlags;
		resource.ReturnType = (gpu_shaderreflection_desc::return_type)inputdesc.ReturnType;
		resource.Dimension = (gpu_shaderreflection_desc::dimension_type)inputdesc.Dimension;
		resource.NumSamples = inputdesc.NumSamples;
		resource.Space = inputdesc.Space;
		resource.uID = inputdesc.uID;
		output.Resources.push_back(resource);
	}

	
	return true;
}

void cfc::gpu_dx12_context::SetStablePowerState(bool isStablePowerState)
{
	m_impl->device->SetStablePowerState(isStablePowerState);
}

u64 cfc::gpu_dx12_context::GetTimeStampFrequency(usize cmdQueueIdx)
{
	u64 timeStampFrequencyTicksPerSecond = 0;
	m_impl->commandQueues[cmdQueueIdx]->GetTimestampFrequency(&timeStampFrequencyTicksPerSecond);
	return timeStampFrequencyTicksPerSecond;
}

void* cfc::gpu_dx12_context::DX12_GetDevice()
{
	return m_impl->device.Get();
}

void* cfc::gpu_dx12_context::DX12_GetCommandList(usize cmdlist)
{
	return m_impl->commandLists[cmdlist].Get();
}

void* cfc::gpu_dx12_context::DX12_GetCommandQueue(usize cmdqueue)
{
	return m_impl->commandQueues[cmdqueue].Get();
}

gpu_copyablefootprint_desc gpu_dx12_context::GetCopyableFootprints(const gpu_resource_desc& resDesc, u32 subresourceIdx /*= 0*/, u64 baseOffset /*= 0*/)
{
	gpu_copyablefootprint_desc ret;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT psf;
	auto res=Convert(resDesc);
	m_impl->device->GetCopyableFootprints(&res, subresourceIdx, 1, baseOffset, &psf, &ret.NumRows, &ret.RowSizeInBytes, &ret.TotalBytes);
	ret.Offset = psf.Offset;
	ret.Width = psf.Footprint.Width;
	ret.Height = psf.Footprint.Height;
	ret.Depth = psf.Footprint.Depth;
	ret.RowPitch = psf.Footprint.RowPitch;
	ret.Format = Convert(psf.Footprint.Format);
	
	return ret;
}



// cmdlist_indirect_api
gpu_dx12_cmdlist_indirect_api::gpu_dx12_cmdlist_indirect_api()
{
	m_ctx = nullptr;
	m_impl.init();
	m_impl->commandSignatureIdx = invalid_index;
	m_impl->needsAlignedStructPadding = false;
	m_templateSizeInBytes = 0;
}

gpu_dx12_cmdlist_indirect_api::gpu_dx12_cmdlist_indirect_api(gpu_dx12_context& ctx)
{
	m_ctx = &ctx;
	m_impl.init();
	m_impl->commandSignatureIdx = invalid_index;
	m_impl->needsAlignedStructPadding = false;
	m_templateSizeInBytes = 0;
}

gpu_dx12_cmdlist_indirect_api::~gpu_dx12_cmdlist_indirect_api()
{
	// command signatures are hidden for the user since they have a really specific use case regarding execute indirect
	if (m_impl->commandSignatureIdx != invalid_index)
	{
		m_ctx->m_impl->commandSignatures.erase(m_impl->commandSignatureIdx);
	}
	m_impl->commandSignatureIdx = invalid_index;
	m_impl.destroy();
}

void gpu_dx12_cmdlist_indirect_api::ICDrawInstanced()
{
	D3D12_INDIRECT_ARGUMENT_DESC descriptor;
	descriptor.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
	m_impl->argumentDescriptor.push_back(descriptor);

	static_assert(sizeof(dx12_indirect_command_descriptors::ICDrawInstanceArgs) == sizeof(D3D12_DRAW_ARGUMENTS), "");
	m_templateSizeInBytes += sizeof(dx12_indirect_command_descriptors::ICDrawInstanceArgs);
}

void gpu_dx12_cmdlist_indirect_api::ICDrawIndexedInstanced()
{
	D3D12_INDIRECT_ARGUMENT_DESC descriptor;
	descriptor.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	m_impl->argumentDescriptor.push_back(descriptor);

	static_assert(sizeof(dx12_indirect_command_descriptors::ICDrawIndexedInstancedArgs) == sizeof(D3D12_DRAW_INDEXED_ARGUMENTS),"");
	m_templateSizeInBytes += sizeof(dx12_indirect_command_descriptors::ICDrawIndexedInstancedArgs);
}

void gpu_dx12_cmdlist_indirect_api::ICDispatch()
{
	D3D12_INDIRECT_ARGUMENT_DESC descriptor;
	descriptor.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
	m_impl->argumentDescriptor.push_back(descriptor);

	static_assert(sizeof(dx12_indirect_command_descriptors::ICDirectDispatchArgs) == sizeof(D3D12_DISPATCH_ARGUMENTS), "");
	m_templateSizeInBytes += sizeof(dx12_indirect_command_descriptors::ICDirectDispatchArgs);
}

void gpu_dx12_cmdlist_indirect_api::ICSetRoot32BitConstants(u32 RootParameterIndex, u32 Num32BitValuesToSet, u32 DestOffsetIn32BitValues)
{
	D3D12_INDIRECT_ARGUMENT_DESC descriptor;
	descriptor.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	descriptor.Constant.RootParameterIndex = RootParameterIndex;
	descriptor.Constant.Num32BitValuesToSet = Num32BitValuesToSet;
	descriptor.Constant.DestOffsetIn32BitValues = DestOffsetIn32BitValues;
	m_impl->argumentDescriptor.push_back(descriptor);

	m_templateSizeInBytes += (sizeof(dx12_indirect_command_descriptors::ICRootConstantArgs) * Num32BitValuesToSet);
}

void gpu_dx12_cmdlist_indirect_api::ICSetRootConstantBufferView(u32 RootParameterIndex)
{
	D3D12_INDIRECT_ARGUMENT_DESC descriptor;
	descriptor.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	descriptor.ConstantBufferView.RootParameterIndex = RootParameterIndex;
	m_impl->argumentDescriptor.push_back(descriptor);

	m_templateSizeInBytes += sizeof(dx12_indirect_command_descriptors::ICRootConstantViewAddress);
}

void gpu_dx12_cmdlist_indirect_api::ICSetRootShaderResourceView(u32 RootParameterIndex)
{
	D3D12_INDIRECT_ARGUMENT_DESC descriptor;
	descriptor.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
	descriptor.ShaderResourceView.RootParameterIndex = RootParameterIndex;
	m_impl->argumentDescriptor.push_back(descriptor);

	m_templateSizeInBytes += sizeof(dx12_indirect_command_descriptors::ICRootConstantViewAddress);
}

void gpu_dx12_cmdlist_indirect_api::ICSetRootUnorderedAccessView(u32 RootParameterIndex)
{
	D3D12_INDIRECT_ARGUMENT_DESC descriptor;
	descriptor.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
	descriptor.UnorderedAccessView.RootParameterIndex = RootParameterIndex;
	m_impl->argumentDescriptor.push_back(descriptor);

	m_templateSizeInBytes += sizeof(dx12_indirect_command_descriptors::ICRootConstantViewAddress);
}

void gpu_dx12_cmdlist_indirect_api::ICIASetVertexBuffers(u32 StartSlot, u32 NumViews)
{
	for (u32 i = 0; i < NumViews; ++i)
	{
		D3D12_INDIRECT_ARGUMENT_DESC descriptor;
		descriptor.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
		descriptor.VertexBuffer.Slot = StartSlot + i;
		m_impl->argumentDescriptor.push_back(descriptor);

		// first argument needs to be aligned to 8 bytes, found by R&D, crash if not compliant
		stl_assert(!(m_templateSizeInBytes & 7));

		static_assert(sizeof(dx12_indirect_command_descriptors::ICVertexBufferViewArgs) == sizeof(D3D12_VERTEX_BUFFER_VIEW), "");
		m_templateSizeInBytes += sizeof(dx12_indirect_command_descriptors::ICVertexBufferViewArgs);
	}

	m_impl->needsAlignedStructPadding = true;
}

void gpu_dx12_cmdlist_indirect_api::ICIASetIndexBuffer()
{
	D3D12_INDIRECT_ARGUMENT_DESC descriptor;
	descriptor.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
	m_impl->argumentDescriptor.push_back(descriptor);

	// first argument needs to be aligned to 8 bytes, found by R&D, crash if not compliant
	stl_assert(!(m_templateSizeInBytes & 7));

	static_assert(sizeof(dx12_indirect_command_descriptors::ICIndexBufferViewArgs) == sizeof(D3D12_INDEX_BUFFER_VIEW), "");
	m_templateSizeInBytes += sizeof(dx12_indirect_command_descriptors::ICIndexBufferViewArgs);

	m_impl->needsAlignedStructPadding = true;
}

bool gpu_dx12_cmdlist_indirect_api::CompileIC(usize RootSignatureIdx, u32 NodeMask)
{
	// cache 
	m_impl->numArgumentsInDescriptor = (u32)m_impl->argumentDescriptor.size();

	// when we use the vertex buffer view or index buffer view, we need to make sure that they stay correctly aligned to 8 bytes when used in N size buffers
	if (m_impl->needsAlignedStructPadding)
	{
		u32 alignmentOffset = m_templateSizeInBytes & 7;
		if (alignmentOffset != 0)
			m_templateSizeInBytes += (8 - alignmentOffset);
	}
		
	// generate command signature descriptor
	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
	commandSignatureDesc.pArgumentDescs = &m_impl->argumentDescriptor[0];
	commandSignatureDesc.NumArgumentDescs = m_impl->numArgumentsInDescriptor;
	commandSignatureDesc.ByteStride = m_templateSizeInBytes; // TODO: align to 8 bytes when 64 bit addresses are used for Index or Vertex buffer view
	commandSignatureDesc.NodeMask = NodeMask;

	// generate command signature
	// since command signatures are specific to execute indirect on DX12, this is hidden from the user for now
	m_impl->commandSignatureIdx = m_ctx->m_impl->commandSignatures.insert();
	
	// root signature is only allowed to be set when there are changes to the root parameters, setting it without these arguments will throw an error
	ID3D12ProxyRootSignature* rootSignature = nullptr;
	if (RootSignatureIdx != invalid_index)
		rootSignature = m_ctx->m_impl->rootSignatures[RootSignatureIdx].Get();

	if (FAILED(m_ctx->m_impl->device->CreateCommandSignature(&commandSignatureDesc, rootSignature, IID_PPV_ARGS(&m_ctx->m_impl->commandSignatures[m_impl->commandSignatureIdx]))))
	{
		m_ctx->m_impl->commandSignatures.erase(m_impl->commandSignatureIdx);
		m_impl->commandSignatureIdx = invalid_index;
		return false;
	}

	// clean argument descriptor, not used anymore
	m_impl->argumentDescriptor.resize(0);

	return true;
}


// cmdlist_bundle_api
#define G(x) reinterpret_cast<ID3D12ProxyGraphicsCommandList*>(x)
gpu_dx12_cmdlist_bundle_api::gpu_dx12_cmdlist_bundle_api(gpu_dx12_context& ctx, usize cmdlistIndex)
{
	m_ctx = &ctx;
	m_cmdlist = ctx.m_impl->commandLists[cmdlistIndex].Get();
}

gpu_dx12_cmdlist_bundle_api::gpu_dx12_cmdlist_bundle_api()
{
	m_ctx = nullptr;
	m_cmdlist = nullptr;
}

void gpu_dx12_cmdlist_bundle_api::ExecuteIndirect(const gpu_dx12_cmdlist_indirect_api* cmdListIndirect, u32 maxCommands, usize argumentBufferIdx, u64 argumentBufferOffset, usize countBufferIdx, u64 countBufferOffset)
{
	ID3D12ProxyCommandSignature* commandSignature = m_ctx->m_impl->commandSignatures[cmdListIndirect->m_impl->commandSignatureIdx].Get();

	ID3D12ProxyResource* argumentBuffer = m_ctx->m_impl->resources[argumentBufferIdx].resource.Get();

	// NOTE: PERF talk NVidia GDC2016 "Advanced Rendering with DirectX11 and DirectX12"
	// NOTE: for best performance in compute, dont use count buffer 
	// NOTE: for best performance in gfx, keep counter buffer count ~max count 
	ID3D12ProxyResource* countBuffer = nullptr;
	if(countBufferIdx != invalid_index)
		countBuffer = m_ctx->m_impl->resources[countBufferIdx].resource.Get();

	G(m_cmdlist)->ExecuteIndirect(commandSignature, maxCommands, argumentBuffer, argumentBufferOffset, countBuffer, countBufferOffset);
}

bool gpu_dx12_cmdlist_bundle_api::Close()
{
	return SUCCEEDED(G(m_cmdlist)->Close());
}

void gpu_dx12_cmdlist_bundle_api::ResolveQueryData(usize queryHeapIdx, gpu_query_type Type, u32 StartIndex, u32 NumQueries, usize destinationResourceIdx, u64 AlignedDestinationBufferOffset)
{
	G(m_cmdlist)->ResolveQueryData(m_ctx->m_impl->queryHeaps[queryHeapIdx].Get(), Convert(Type), StartIndex, NumQueries, m_ctx->m_impl->resources[destinationResourceIdx].resource.Get(), AlignedDestinationBufferOffset);
}

void gpu_dx12_cmdlist_bundle_api::IASetVertexBuffers(u32 StartSlot, u32 NumViews, const gpu_vertexbuffer_view *pViews)
{
	G(m_cmdlist)->IASetVertexBuffers(StartSlot, NumViews, (D3D12_VERTEX_BUFFER_VIEW*)pViews);
}

void gpu_dx12_cmdlist_bundle_api::IASetIndexBuffer(u64 gpuBufferLocation, u32 sizeInBytes, gpu_format_type format)
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = gpuBufferLocation;
	ibv.Format = Convert(format);
	ibv.SizeInBytes = sizeInBytes;
	G(m_cmdlist)->IASetIndexBuffer(&ibv);
}

void gpu_dx12_cmdlist_bundle_api::SetGraphicsRootUnorderedAccessView(u32 RootParameterIndex, u64 gpuBufferLocation)
{
	G(m_cmdlist)->SetGraphicsRootUnorderedAccessView(RootParameterIndex, gpuBufferLocation);
}

void gpu_dx12_cmdlist_bundle_api::SetComputeRootUnorderedAccessView(u32 RootParameterIndex, u64 gpuBufferLocation)
{
	G(m_cmdlist)->SetComputeRootUnorderedAccessView(RootParameterIndex, gpuBufferLocation);
}

void gpu_dx12_cmdlist_bundle_api::SetGraphicsRootShaderResourceView(u32 RootParameterIndex, u64 gpuBufferLocation)
{
	G(m_cmdlist)->SetGraphicsRootShaderResourceView(RootParameterIndex, gpuBufferLocation);
}

void gpu_dx12_cmdlist_bundle_api::SetComputeRootShaderResourceView(u32 RootParameterIndex, u64 gpuBufferLocation)
{
	G(m_cmdlist)->SetComputeRootShaderResourceView(RootParameterIndex, gpuBufferLocation);
}

void gpu_dx12_cmdlist_bundle_api::SetGraphicsRootConstantBufferView(u32 RootParameterIndex, u64 gpuBufferLocation)
{
	G(m_cmdlist)->SetGraphicsRootConstantBufferView(RootParameterIndex, gpuBufferLocation);
}

void gpu_dx12_cmdlist_bundle_api::SetComputeRootConstantBufferView(u32 RootParameterIndex, u64 gpuBufferLocation)
{
	G(m_cmdlist)->SetComputeRootConstantBufferView(RootParameterIndex, gpuBufferLocation);
}

void gpu_dx12_cmdlist_bundle_api::SetGraphicsRoot32BitConstants(u32 RootParameterIndex, u32 Num32BitValuesToSet, const void *pSrcData, u32 DestOffsetIn32BitValues)
{
	G(m_cmdlist)->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
}

void gpu_dx12_cmdlist_bundle_api::SetComputeRoot32BitConstants(u32 RootParameterIndex, u32 Num32BitValuesToSet, const void *pSrcData, u32 DestOffsetIn32BitValues)
{
	G(m_cmdlist)->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
}

void gpu_dx12_cmdlist_bundle_api::SetGraphicsRoot32BitConstant(u32 RootParameterIndex, u32 SrcData, u32 DestOffsetIn32BitValues)
{
	G(m_cmdlist)->SetGraphicsRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
}

void gpu_dx12_cmdlist_bundle_api::SetComputeRoot32BitConstant(u32 RootParameterIndex, u32 SrcData, u32 DestOffsetIn32BitValues)
{
	G(m_cmdlist)->SetComputeRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
}

void gpu_dx12_cmdlist_bundle_api::SetGraphicsRootDescriptorTable(u32 RootParameterIndex, u64 gpuBaseDescriptor)
{
	G(m_cmdlist)->SetGraphicsRootDescriptorTable(RootParameterIndex, ConvertGPU(gpuBaseDescriptor));
}

void gpu_dx12_cmdlist_bundle_api::SetComputeRootDescriptorTable(u32 RootParameterIndex, u64 gpuBaseDescriptor)
{
	G(m_cmdlist)->SetComputeRootDescriptorTable(RootParameterIndex, ConvertGPU(gpuBaseDescriptor));
}

void gpu_dx12_cmdlist_bundle_api::SetGraphicsRootSignature(usize rootSignatureIdx)
{
	G(m_cmdlist)->SetGraphicsRootSignature(m_ctx->m_impl->rootSignatures[rootSignatureIdx].Get());
}

void gpu_dx12_cmdlist_bundle_api::SetComputeRootSignature(usize rootSignatureIdx)
{
	G(m_cmdlist)->SetComputeRootSignature(m_ctx->m_impl->rootSignatures[rootSignatureIdx].Get());
}

void gpu_dx12_cmdlist_bundle_api::SetDescriptorHeaps(u32 NumDescriptorHeaps, const usize* descriptorHeapIdxArray)
{
	ID3D12ProxyDescriptorHeap** heaps = NumDescriptorHeaps == 0 ? nullptr : (ID3D12ProxyDescriptorHeap**)_alloca(sizeof(ID3D12ProxyDescriptorHeap*)*NumDescriptorHeaps);
	for (u32 i = 0; i < NumDescriptorHeaps; i++)
		heaps[i] = m_ctx->m_impl->descriptorHeaps[descriptorHeapIdxArray[i]].Get();
	G(m_cmdlist)->SetDescriptorHeaps(NumDescriptorHeaps, heaps);
}

void gpu_dx12_cmdlist_bundle_api::SetPipelineState(usize pipelineStateIdx)
{
	G(m_cmdlist)->SetPipelineState(m_ctx->m_impl->pipelineStates[pipelineStateIdx].Get());
}

void gpu_dx12_cmdlist_bundle_api::OMSetStencilRef(u32 StencilRef)
{
	G(m_cmdlist)->OMSetStencilRef(StencilRef);
}

void gpu_dx12_cmdlist_bundle_api::OMSetBlendFactor(const f32 BlendFactor[4])
{
	G(m_cmdlist)->OMSetBlendFactor(BlendFactor);
}

void gpu_dx12_cmdlist_bundle_api::IASetPrimitiveTopology(gpu_primitive_type PrimitiveTopology)
{
	G(m_cmdlist)->IASetPrimitiveTopology((D3D12_PRIMITIVE_TOPOLOGY)PrimitiveTopology);
}

void gpu_dx12_cmdlist_bundle_api::Dispatch(u32 ThreadGroupCountX, u32 ThreadGroupCountY, u32 ThreadGroupCountZ)
{
	G(m_cmdlist)->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void gpu_dx12_cmdlist_bundle_api::DrawIndexedInstanced(u32 IndexCountPerInstance, u32 InstanceCount, u32 StartIndexLocation, i32 BaseVertexLocation, u32 StartInstanceLocation)
{
	G(m_cmdlist)->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void gpu_dx12_cmdlist_bundle_api::DrawInstanced(u32 VertexCountPerInstance, u32 InstanceCount, u32 StartVertexLocation, u32 StartInstanceLocation)
{
	G(m_cmdlist)->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

bool gpu_dx12_cmdlist_bundle_api::Reset(usize cmdAllocatorIdx, usize pipelineStateIdx)
{
	return SUCCEEDED(G(m_cmdlist)->Reset(m_ctx->m_impl->commandAllocators[cmdAllocatorIdx].Get(), pipelineStateIdx == invalid_index ? nullptr : m_ctx->m_impl->pipelineStates[pipelineStateIdx].Get()));
}


// cmdlist_direct_api inherits from cmdlist_bundle_api
gpu_dx12_cmdlist_direct_api::gpu_dx12_cmdlist_direct_api(gpu_dx12_context& ctx, usize cmdlistIndex)
	:
	gpu_dx12_cmdlist_bundle_api(ctx, cmdlistIndex)
{
}

gpu_dx12_cmdlist_direct_api::gpu_dx12_cmdlist_direct_api()
	:
	gpu_dx12_cmdlist_bundle_api()
{
}

void gpu_dx12_cmdlist_direct_api::SetPredication(usize resourceIdx, u64 AlignedBufferOffset, bool OpEqualZero)
{
	G(m_cmdlist)->SetPredication(m_ctx->m_impl->resources[resourceIdx].resource.Get(), AlignedBufferOffset, OpEqualZero ? D3D12_PREDICATION_OP_EQUAL_ZERO : D3D12_PREDICATION_OP_NOT_EQUAL_ZERO);
}

void gpu_dx12_cmdlist_direct_api::EndQuery(usize queryHeapIdx, gpu_query_type Type, u32 Index)
{
	G(m_cmdlist)->EndQuery(m_ctx->m_impl->queryHeaps[queryHeapIdx].Get(), Convert(Type), Index);
}

void gpu_dx12_cmdlist_direct_api::BeginQuery(usize queryHeapIdx, gpu_query_type Type, u32 Index)
{
	G(m_cmdlist)->BeginQuery(m_ctx->m_impl->queryHeaps[queryHeapIdx].Get(), Convert(Type), Index);
}

void gpu_dx12_cmdlist_direct_api::DiscardResource(usize resourceIdx, u32 numRects, const gpu_rectangle *pRects, u32 firstSubresource, u32 numSubresources)
{
	D3D12_DISCARD_REGION region;
	region.NumRects = numRects;
	region.pRects = (D3D12_RECT *)pRects;
	region.FirstSubresource = firstSubresource;
	region.NumSubresources = numSubresources;
	G(m_cmdlist)->DiscardResource(m_ctx->m_impl->resources[resourceIdx].resource.Get(), &region);
}

void gpu_dx12_cmdlist_direct_api::DiscardResource(usize resourceIdx)
{
	G(m_cmdlist)->DiscardResource(m_ctx->m_impl->resources[resourceIdx].resource.Get(), nullptr);
}

void gpu_dx12_cmdlist_direct_api::ClearUnorderedAccessViewFloat(u64 gpuViewGPUHandleInCurrentHeap, usize cpuViewCPUHandle, usize resourceIdx, const f32 Values[4], u32 NumRects, const gpu_rectangle *pRects)
{
	G(m_cmdlist)->ClearUnorderedAccessViewFloat(ConvertGPU(gpuViewGPUHandleInCurrentHeap), ConvertCPU(cpuViewCPUHandle), m_ctx->m_impl->resources[resourceIdx].resource.Get(), Values, NumRects, (D3D12_RECT*)pRects);
}

void gpu_dx12_cmdlist_direct_api::ClearUnorderedAccessViewUint(u64 gpuViewGPUHandleInCurrentHeap, usize cpuViewCPUHandle, usize resourceIdx, const u32 Values[4], u32 NumRects, const gpu_rectangle *pRects)
{
	G(m_cmdlist)->ClearUnorderedAccessViewUint(ConvertGPU(gpuViewGPUHandleInCurrentHeap), ConvertCPU(cpuViewCPUHandle), m_ctx->m_impl->resources[resourceIdx].resource.Get(), Values, NumRects, (D3D12_RECT*)pRects);
}

void gpu_dx12_cmdlist_direct_api::ClearRenderTargetView(usize cpuRenderTargetView, const f32 ColorRGBA[4], u32 NumRects, const gpu_rectangle *pRects)
{
	G(m_cmdlist)->ClearRenderTargetView(ConvertCPU(cpuRenderTargetView), ColorRGBA, NumRects, (D3D12_RECT*)pRects);
}

void gpu_dx12_cmdlist_direct_api::ClearDepthStencilView(usize cpuDepthStencilView, bool clearDepth, bool clearStencil, f32 Depth, u8 Stencil, u32 NumRects, const gpu_rectangle *pRects)
{
	int flags = (clearDepth ? D3D12_CLEAR_FLAG_DEPTH : 0) | (clearStencil ? D3D12_CLEAR_FLAG_STENCIL : 0);
	G(m_cmdlist)->ClearDepthStencilView(ConvertCPU(cpuDepthStencilView), (D3D12_CLEAR_FLAGS)flags, Depth, Stencil, NumRects, (D3D12_RECT *)pRects);
}

void gpu_dx12_cmdlist_direct_api::OMSetRenderTargets(u32 NumRenderTargetDescriptors, const usize* cpuRenderTargetDescriptors, bool RTsSingleHandleToDescriptorRange, const usize cpuDepthStencilDescriptor)
{
	G(m_cmdlist)->OMSetRenderTargets(NumRenderTargetDescriptors, (D3D12_CPU_DESCRIPTOR_HANDLE*)cpuRenderTargetDescriptors, RTsSingleHandleToDescriptorRange ? TRUE : FALSE, cpuDepthStencilDescriptor == invalid_index ? nullptr : (D3D12_CPU_DESCRIPTOR_HANDLE*)&cpuDepthStencilDescriptor);
}

void gpu_dx12_cmdlist_direct_api::SOSetTargets(u32 StartSlot, u32 NumViews, const gpu_streamout_view *pViews)
{
	G(m_cmdlist)->SOSetTargets(StartSlot, NumViews, (D3D12_STREAM_OUTPUT_BUFFER_VIEW*)pViews);
}

void gpu_dx12_cmdlist_direct_api::ExecuteBundle(usize bundleCmdListIdx)
{
	G(m_cmdlist)->ExecuteBundle((ID3D12ProxyGraphicsCommandList*)m_ctx->m_impl->commandLists[bundleCmdListIdx].Get());
}

void gpu_dx12_cmdlist_direct_api::ResourceBarrier(u32 NumBarriers, const gpu_resourcebarrier_desc *pBarriers)
{
	D3D12_PROXY_RESOURCE_BARRIER* barriers = (D3D12_PROXY_RESOURCE_BARRIER*)_alloca(NumBarriers*sizeof(D3D12_PROXY_RESOURCE_BARRIER));
	for (u32 i = 0;i < NumBarriers; i++)
	{
		barriers[i].Type = (D3D12_RESOURCE_BARRIER_TYPE) pBarriers[i].Type;
		barriers[i].Flags = (D3D12_RESOURCE_BARRIER_FLAGS)pBarriers[i].Flags;
		switch (barriers[i].Type)
		{
		case D3D12_RESOURCE_BARRIER_TYPE_TRANSITION:
			barriers[i].Transition.pResource = m_ctx->m_impl->resources[pBarriers[i].BarrierTransition.ResourceIdx].resource.Get();
			barriers[i].Transition.StateAfter = (D3D12_RESOURCE_STATES) pBarriers[i].BarrierTransition.StateAfter;
			barriers[i].Transition.StateBefore = (D3D12_RESOURCE_STATES)pBarriers[i].BarrierTransition.StateBefore;
			barriers[i].Transition.Subresource = pBarriers[i].BarrierTransition.Subresource;
			break;
		case D3D12_RESOURCE_BARRIER_TYPE_ALIASING:
			barriers[i].Aliasing.pResourceBefore = m_ctx->m_impl->resources[pBarriers[i].BarrierAliasing.ResourceBeforeIdx].resource.Get();
			barriers[i].Aliasing.pResourceAfter = m_ctx->m_impl->resources[pBarriers[i].BarrierAliasing.ResourceAfterIdx].resource.Get();
			break;
		case D3D12_RESOURCE_BARRIER_TYPE_UAV:
			barriers[i].UAV.pResource = m_ctx->m_impl->resources[pBarriers[i].BarrierUAV.ResourceIdx].resource.Get();
			break;
		}
	}
	G(m_cmdlist)->ResourceBarrier(NumBarriers, barriers);
}

void gpu_dx12_cmdlist_direct_api::RSSetScissorRects(u32 NumRects, const gpu_rectangle *pRects)
{
	stl_assert(m_cmdlist != nullptr);
	G(m_cmdlist)->RSSetScissorRects(NumRects, (D3D12_RECT*)pRects);
}

void gpu_dx12_cmdlist_direct_api::RSSetViewports(u32 NumViewports, const gpu_viewport *pViewports)
{
	stl_assert(m_cmdlist != nullptr);
	G(m_cmdlist)->RSSetViewports(NumViewports, (D3D12_VIEWPORT*)pViewports);
}

void gpu_dx12_cmdlist_direct_api::ResolveSubresource(usize pDstResource, u32 DstSubresource, usize pSrcResource, u32 SrcSubresource, gpu_format_type Format)
{
	stl_assert(m_cmdlist != nullptr);
	G(m_cmdlist)->ResolveSubresource(m_ctx->m_impl->resources[pDstResource].resource.Get(), DstSubresource, m_ctx->m_impl->resources[pSrcResource].resource.Get(), SrcSubresource, Convert(Format));
}

void gpu_dx12_cmdlist_direct_api::CopyResource(usize pDstResource, usize pSrcResource)
{
	stl_assert(m_cmdlist != nullptr);
	G(m_cmdlist)->CopyResource(m_ctx->m_impl->resources[pDstResource].resource.Get(), m_ctx->m_impl->resources[pSrcResource].resource.Get());
}

void gpu_dx12_cmdlist_direct_api::CopyBufferRegion(usize pDstBuffer, u64 DstOffset, usize pSrcBuffer, u64 SrcOffset, u64 NumBytes)
{
	stl_assert(m_cmdlist != nullptr);
	G(m_cmdlist)->CopyBufferRegion(m_ctx->m_impl->resources[pDstBuffer].resource.Get(), DstOffset, m_ctx->m_impl->resources[pSrcBuffer].resource.Get(), SrcOffset, NumBytes);
}

void gpu_dx12_cmdlist_direct_api::CopyTextureRegion(const gpu_texturecopy_desc& pDst, const gpu_texturecopy_desc& pSrc, u32 DstX/*=0*/, u32 DstY/*=0*/, u32 DstZ/*=0*/, const gpu_box *pSrcBox/*=nullptr*/)
{
	D3D12_PROXY_TEXTURE_COPY_LOCATION locationDst = Convert(pDst, m_ctx->m_impl->resources[pDst.ResourceIdx].resource.Get());
	D3D12_PROXY_TEXTURE_COPY_LOCATION locationSrc = Convert(pSrc, m_ctx->m_impl->resources[pSrc.ResourceIdx].resource.Get());
	
	stl_assert(m_cmdlist != nullptr);
	G(m_cmdlist)->CopyTextureRegion(&locationDst, DstX, DstY, DstZ, &locationSrc, (D3D12_BOX*)pSrcBox);
}

void gpu_dx12_cmdlist_direct_api::ClearState(usize pipelineStateIdx)
{
	stl_assert(m_cmdlist != nullptr);
	G(m_cmdlist)->ClearState(m_ctx->m_impl->pipelineStates[pipelineStateIdx].Get());
}

#undef G


};