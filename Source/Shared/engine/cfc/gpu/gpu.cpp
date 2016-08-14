#include "gpu.h"
#include "math.h"
#include <cfc/math/math.h>

namespace cfc
{
	gpu_resourcebarrier_desc gpu_resourcebarrier_desc::Transition(usize resourceIdx, gpu_resourcestate::flag stateBefore, gpu_resourcestate::flag stateAfter, u32 subResource, gpu_resourcebarrierflag_type flags)
	{
		gpu_resourcebarrier_desc ret;
		ret.Type = gpu_resourcebarrier_type::Transition;
		ret.Flags = flags;
		ret.BarrierTransition.ResourceIdx = resourceIdx;
		ret.BarrierTransition.StateBefore = stateBefore;
		ret.BarrierTransition.StateAfter = stateAfter;
		ret.BarrierTransition.Subresource = subResource;
		return ret;
	}

	gpu_resourcebarrier_desc gpu_resourcebarrier_desc::Aliasing(usize resourceBeforeIdx, usize resourceAfterIdx, gpu_resourcebarrierflag_type flags)
	{
		gpu_resourcebarrier_desc ret;
		ret.Type = gpu_resourcebarrier_type::Aliasing;
		ret.Flags = flags;
		ret.BarrierAliasing.ResourceBeforeIdx = resourceBeforeIdx;
		ret.BarrierAliasing.ResourceAfterIdx = resourceAfterIdx;
		return ret;
	}

	gpu_resourcebarrier_desc gpu_resourcebarrier_desc::UAV(usize resourceIdx, gpu_resourcebarrierflag_type flags)
	{
		gpu_resourcebarrier_desc ret;
		ret.Type = gpu_resourcebarrier_type::Uav;
		ret.Flags = flags;
		ret.BarrierUAV.ResourceIdx = resourceIdx;
		return ret;
	}

	gpu_resource_desc::gpu_resource_desc(dimension a_dimension, u64 alignment, u64 width, u32 height, u16 depth, u16 mipLevels, gpu_format_type a_format, u32 sampleCount, u32 sampleQuality, texturelayout layout, resourceflags::flag flags) :
		Dimension(a_dimension), Alignment(alignment), Width(width), Height(height), DepthOrArraySize(depth), MipLevels(mipLevels), Format(a_format), SampleCount(sampleCount), SampleQuality(sampleQuality), Layout(layout), Flags(flags)
	{

	}

	gpu_resource_desc::gpu_resource_desc()
	{

	}

	gpu_resource_desc gpu_resource_desc::Buffer(u64 width, resourceflags::flag flags /*= resourceflags::None*/, u64 alignment /*= 0*/)
	{
		return gpu_resource_desc(dimension::Buffer, alignment, width, 1, 1, 1, gpu_format_type::Unknown, 1, 0, texturelayout::RowMajor, flags);
	}

	gpu_resource_desc gpu_resource_desc::Tex1D(gpu_format_type fmt, u64 width, u16 arraySize /*= 1*/, u16 mipLevels /*= 0*/, resourceflags::flag flags /*= resourceflags::None*/, texturelayout layout /*= texturelayout::Unknown*/, u64 alignment /*= 0*/)
	{
		return gpu_resource_desc(dimension::Texture1D, alignment, width, 1, arraySize, mipLevels, fmt, 1, 0, layout, flags);
	}

	gpu_resource_desc gpu_resource_desc::Tex2D(gpu_format_type fmt, u64 width, u32 height, u16 arraySize /*= 1*/, u16 mipLevels /*= 0*/, resourceflags::flag flags /*= resourceflags::None*/, texturelayout layout /*= texturelayout::Unknown*/, u64 alignment /*= 0*/)
	{
		return gpu_resource_desc(dimension::Texture2D, alignment, width, height, arraySize, mipLevels, fmt, 1, 0, layout, flags);
	}

	gpu_resource_desc gpu_resource_desc::Tex3D(gpu_format_type fmt, u64 width, u32 height, u16 depth, u16 mipLevels /*= 0*/, resourceflags::flag flags /*= resourceflags::None*/, texturelayout layout /*= texturelayout::Unknown*/, u64 alignment /*= 0*/)
	{
		return gpu_resource_desc(dimension::Texture3D, alignment, width, height, depth, mipLevels, fmt, 1, 0, layout, flags);
	}

	gpu_texturecopy_desc gpu_texturecopy_desc::AsPlacedFootprint(usize idx, const gpu_copyablefootprint_desc& desc)
	{
		gpu_texturecopy_desc ret;
		ret.ResourceIdx = idx;
		ret.Type = copytype::PlacedFootprint;
		ret.PlacedFootprint = desc;
		return ret;

	}

	gpu_texturecopy_desc gpu_texturecopy_desc::AsSubresourceIndex(usize idx, u32 subresourceIndex /*= 0*/)
	{
		gpu_texturecopy_desc ret;
		ret.ResourceIdx = idx;
		ret.Type = copytype::SubresourceIndex;
		ret.SubresourceIndex = subresourceIndex;
		return ret;
	}

	u32 gpu_shaderreflection_desc::parameter_desc::ConvertMaskToNumChannels() const
	{
		// fast switch table
		switch (Mask)
		{
		case 0x0:
			return 0;
		case 0x1: // binary: 0001
			return 1;
		case 0x3: // binary: 0011
			return 2;
		case 0x7: // binary: 0111
			return 3;
		case 0xf: // binary: 1111
			return 4;
		default:
		{
			// slow path: count number of bits in channel for other values
			u32 channels = 0;
			u32 msk = Mask;
			while (msk > 0)
			{
				msk >>= 1;
				++channels;
			}
			return channels;
		}
		};

	}

	bool gpu_format_type_query::IsDepthType(gpu_format_type fmt)
	{
		switch (fmt)
		{
		case gpu_format_type::D16Unorm: return true;
		case gpu_format_type::D24UnormS8Uint: return true;
		case gpu_format_type::D32Float: return true;
		case gpu_format_type::D32FloatS8X24Uint: return true;
		default: return false;
		}
	}


	bool gpu_format_type_query::IsStencilType(gpu_format_type fmt)
	{
		switch (fmt)
		{
		case gpu_format_type::D24UnormS8Uint: return true;
		case gpu_format_type::D32FloatS8X24Uint: return true;
		default: return false;
		}
	}

	bool gpu_format_type_query::IsCompressedType(gpu_format_type fmt)
	{
		switch (fmt)
		{
		case gpu_format_type::BC1Typeless: return true;
		case gpu_format_type::BC1Unorm: return true;
		case gpu_format_type::BC1UnormSrgb: return true;
		case gpu_format_type::BC2Typeless: return true;
		case gpu_format_type::BC2Unorm: return true;
		case gpu_format_type::BC2UnormSrgb: return true;
		case gpu_format_type::BC3Typeless: return true;
		case gpu_format_type::BC3Unorm: return true;
		case gpu_format_type::BC3UnormSrgb: return true;
		case gpu_format_type::BC4Snorm: return true;
		case gpu_format_type::BC4Typeless: return true;
		case gpu_format_type::BC4Unorm: return true;
		case gpu_format_type::BC5Snorm: return true;
		case gpu_format_type::BC5Typeless: return true;
		case gpu_format_type::BC5Unorm: return true;
		case gpu_format_type::BC6H_SF16: return true;
		case gpu_format_type::BC6H_UF16: return true;
		case gpu_format_type::BC6HTypeless: return true;
		case gpu_format_type::BC7Typeless: return true;
		case gpu_format_type::BC7Unorm: return true;
		case gpu_format_type::BC7UnormSrgb: return true;
		};
		return false;
	}

	i32 gpu_format_type_query::GetBitsPerChannel(gpu_format_type fmt)
	{
		static i32 table[] = { -1,32,32,32,32,32,32,32,32,16,16,16,16,16,16,32,32,32,32,32,32,32,32,32,32,32,32,8,8,8,8,8,8,16,16,16,16,16,16,32,32,32,32,32,32,32,32,32,8,8,8,8,8,16,16,16,16,16,16,16,8,8,8,8,8,8,1,32,8,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,16,16,8,8,32,8,8,8,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,4,-1,-1,-1,-1 };
		return table[(int)fmt];
	}

	i32 gpu_format_type_query::GetNumChannels(gpu_format_type fmt)
	{
		static i32 table[] = { -1,4,4,4,4,3,3,3,3,4,4,4,4,4,4,2,2,2,2,2,2,2,2,1,1,1,1,4,4,4,4,4,4,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,4,4,1,4,4,4,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,4,-1,-1,-1,-1 };
		return table[(int)fmt];
	}

	i32 gpu_format_type_query::GetCompressedBlockSize(gpu_format_type fmt)
	{
		static i32 table[] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,8,8,8,8,8,16,16,16,8,8,8,16,16,16,-1,-1,-1,-1,-1,-1,-1,-1,-1,16,16,16,16,16,16,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
		return table[(int)fmt];
	}

	i32 gpu_format_type_query::GetBitsPerPixel(gpu_format_type fmt)
	{
		i32 channels = GetBitsPerChannel(fmt);
		if (channels < 0)
			return 0;
		return channels * GetBitsPerPixel(fmt);
	}


	usize gpu_format_type_query::GetRowBytes(gpu_format_type fmt, usize width)
	{
		if (IsCompressedType(fmt))
			return MAX(1u, ((width + 3) / 4)) * GetCompressedBlockSize(fmt);
		else
			return (width * GetBitsPerPixel(fmt) + 7) / 8;
	}

}; // end namespace cfc

