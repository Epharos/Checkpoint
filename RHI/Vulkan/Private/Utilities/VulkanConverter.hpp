#pragma once

#include "../pch.hpp"

#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>
#include <RHI/Utilities.hpp>

namespace cp
{
	template<>
	inline vk::Format EnumCast<vk::Format, Format>(const Format _format)
	{
		switch (_format)
		{
			// Color formats
			case Format::R8G8B8A8_UNORM: return vk::Format::eR8G8B8A8Unorm;
			case Format::B8G8R8A8_UNORM: return vk::Format::eB8G8R8A8Unorm;
			case Format::R16G16B16A16_FLOAT: return vk::Format::eR16G16B16A16Sfloat;
			case Format::R32_UINT: return vk::Format::eR32Uint;

			// Depth formats
			case Format::D24_UNORM_S8_UINT: return vk::Format::eD24UnormS8Uint;
			case Format::D32_FLOAT: return vk::Format::eD32Sfloat;

			default: throw std::logic_error("Unrecognized format!");
		}
	}

	template<>
	inline Format EnumCast<Format, vk::Format>(const vk::Format _format)
	{
		switch (_format)
		{
			// Color formats
			case vk::Format::eR8G8B8A8Unorm: return Format::R8G8B8A8_UNORM;
			case vk::Format::eB8G8R8A8Unorm: return Format::B8G8R8A8_UNORM;
			case vk::Format::eR16G16B16A16Sfloat: return Format::R16G16B16A16_FLOAT;
			case vk::Format::eR32Uint: return Format::R32_UINT;

			// Depth formats
			case vk::Format::eD24UnormS8Uint: return Format::D24_UNORM_S8_UINT;
			case vk::Format::eD32Sfloat: return Format::D32_FLOAT;

			default: throw std::logic_error("Unrecognized format!");
		}
	}

	template<>
	inline vk::ImageUsageFlagBits EnumCast<vk::ImageUsageFlagBits, TextureUsage>(const TextureUsage _usage)
	{
		switch (_usage)
		{
			case TextureUsage::Sampled: return vk::ImageUsageFlagBits::eSampled;
			case TextureUsage::ColorAttachment: return vk::ImageUsageFlagBits::eColorAttachment;
			case TextureUsage::DepthStencilAttachment: return vk::ImageUsageFlagBits::eDepthStencilAttachment;
			case TextureUsage::Storage: return vk::ImageUsageFlagBits::eStorage;
			case TextureUsage::TransferSrc: return vk::ImageUsageFlagBits::eTransferSrc;
			case TextureUsage::TransferDst: return vk::ImageUsageFlagBits::eTransferDst;
		}

		throw std::logic_error("Unrecognized texture usage!");
	}

	template<>
	inline vk::ImageUsageFlags EnumBitsCast<vk::ImageUsageFlags, TextureUsage>(const TextureUsage _usage)
	{
		return ConvertBitField<vk::ImageUsageFlags, vk::ImageUsageFlagBits, TextureUsage, 6>(_usage);
	}

	template<>
	inline vk::ImageAspectFlagBits EnumCast<vk::ImageAspectFlagBits, TextureAspect>(const TextureAspect _aspect)
	{
		switch (_aspect)
		{
			case TextureAspect::None: return vk::ImageAspectFlagBits::eNone;
			case TextureAspect::Color: return vk::ImageAspectFlagBits::eColor;
			case TextureAspect::Depth: return vk::ImageAspectFlagBits::eDepth;
			case TextureAspect::Stencil: return vk::ImageAspectFlagBits::eStencil;
			default: throw std::logic_error("Unrecognized texture aspect!");
		}
	}

	template<>
	inline vk::ImageAspectFlags EnumBitsCast<vk::ImageAspectFlags, TextureAspect>(const TextureAspect _aspect)
	{
		return ConvertBitField<vk::ImageAspectFlags, vk::ImageAspectFlagBits, TextureAspect, 4>(_aspect);
	}

	template<>
	inline vk::ImageLayout EnumCast<vk::ImageLayout, TextureLayout>(const TextureLayout _layout)
	{
		switch (_layout)
		{
			case TextureLayout::Undefined: return vk::ImageLayout::eUndefined;
			case TextureLayout::General: return vk::ImageLayout::eGeneral;
			case TextureLayout::AttachmentOptimal: return vk::ImageLayout::eAttachmentOptimal;
			case TextureLayout::ColorAttachment: return vk::ImageLayout::eColorAttachmentOptimal;
			case TextureLayout::DepthStencilAttachment: return vk::ImageLayout::eDepthStencilAttachmentOptimal;
			case TextureLayout::DepthStencilReadOnly: return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
			case TextureLayout::ShaderReadOnly: return vk::ImageLayout::eShaderReadOnlyOptimal;
			case TextureLayout::ResolveSrc:
			case TextureLayout::TransferSrc: return vk::ImageLayout::eTransferSrcOptimal;
			case TextureLayout::ResolveDst:
			case TextureLayout::TransferDst: return vk::ImageLayout::eTransferDstOptimal;
			case TextureLayout::Present: return vk::ImageLayout::ePresentSrcKHR;
			default: throw std::logic_error("Unrecognized texture layout!");
		}
	}

	template<>
	inline vk::ImageViewType EnumCast<vk::ImageViewType, TextureType>(const TextureType _type)
	{
		switch (_type)
		{
			case TextureType::Texture2D: return vk::ImageViewType::e2D;
			case TextureType::Texture3D: return vk::ImageViewType::e3D;
			case TextureType::Texture2DArray: return vk::ImageViewType::e2DArray;
			case TextureType::TextureCube: return vk::ImageViewType::eCube;
			default: throw std::logic_error("Unrecognized texture type!");
		}
	}

	template<>
	inline vk::AttachmentLoadOp EnumCast<vk::AttachmentLoadOp, LoadOp>(const LoadOp _loadOp)
	{
		switch (_loadOp)
		{
			case LoadOp::Clear: return vk::AttachmentLoadOp::eClear;
			case LoadOp::Load: return vk::AttachmentLoadOp::eLoad;
			case LoadOp::DontCare: return vk::AttachmentLoadOp::eDontCare;
			case LoadOp::None: return vk::AttachmentLoadOp::eNone;
			default: throw std::logic_error("Unrecognized load op!");
		}
	}

	template<>
	inline vk::AttachmentStoreOp EnumCast<vk::AttachmentStoreOp, StoreOp>(const StoreOp _storeOp)
	{
		switch (_storeOp)
		{
			case StoreOp::Store: return vk::AttachmentStoreOp::eStore;
			case StoreOp::DontCare: return vk::AttachmentStoreOp::eDontCare;
			case StoreOp::None: return vk::AttachmentStoreOp::eNone;
			default: throw std::logic_error("Unrecognized store op!");
		}
	}

	template<>
	inline vk::PipelineStageFlagBits EnumCast<vk::PipelineStageFlagBits, PipelineStage>(const PipelineStage _stage)
	{
		switch (_stage)
		{
			case PipelineStage::Top: return vk::PipelineStageFlagBits::eTopOfPipe;
			case PipelineStage::DrawIndirect: return vk::PipelineStageFlagBits::eDrawIndirect;
			case PipelineStage::VertexInput: return vk::PipelineStageFlagBits::eVertexInput;
			case PipelineStage::VertexShader: return vk::PipelineStageFlagBits::eVertexShader;
			case PipelineStage::FragmentShader: return vk::PipelineStageFlagBits::eFragmentShader;
			case PipelineStage::EarlyDepth: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
			case PipelineStage::LateDepth: return vk::PipelineStageFlagBits::eLateFragmentTests;
			case PipelineStage::ColorAttachment: return vk::PipelineStageFlagBits::eColorAttachmentOutput;
			case PipelineStage::ComputeShader: return vk::PipelineStageFlagBits::eComputeShader;
			case PipelineStage::Transfer: return vk::PipelineStageFlagBits::eTransfer;
			case PipelineStage::Bottom: return vk::PipelineStageFlagBits::eBottomOfPipe;
			case PipelineStage::AllGraphics: return vk::PipelineStageFlagBits::eAllGraphics;
			case PipelineStage::AllCommands: return vk::PipelineStageFlagBits::eAllCommands;
			default: throw std::logic_error("Unrecognized pipeline stage!");
		}
	}

	template<>
	inline vk::PipelineStageFlags EnumBitsCast<vk::PipelineStageFlags, PipelineStage>(const PipelineStage _stage)
	{
		return ConvertBitField<vk::PipelineStageFlags, vk::PipelineStageFlagBits, PipelineStage, 13>(_stage);
	}

	template<>
	inline vk::PipelineStageFlagBits2 EnumCast<vk::PipelineStageFlagBits2, PipelineStage>(const PipelineStage _stage)
	{
		switch (_stage)
		{
			case PipelineStage::Top: return vk::PipelineStageFlagBits2::eTopOfPipe;
			case PipelineStage::DrawIndirect: return vk::PipelineStageFlagBits2::eDrawIndirect;
			case PipelineStage::VertexInput: return vk::PipelineStageFlagBits2::eVertexInput;
			case PipelineStage::VertexShader: return vk::PipelineStageFlagBits2::eVertexShader;
			case PipelineStage::FragmentShader: return vk::PipelineStageFlagBits2::eFragmentShader;
			case PipelineStage::EarlyDepth: return vk::PipelineStageFlagBits2::eEarlyFragmentTests;
			case PipelineStage::LateDepth: return vk::PipelineStageFlagBits2::eLateFragmentTests;
			case PipelineStage::ColorAttachment: return vk::PipelineStageFlagBits2::eColorAttachmentOutput;
			case PipelineStage::ComputeShader: return vk::PipelineStageFlagBits2::eComputeShader;
			case PipelineStage::Transfer: return vk::PipelineStageFlagBits2::eTransfer;
			case PipelineStage::Bottom: return vk::PipelineStageFlagBits2::eBottomOfPipe;
			case PipelineStage::AllGraphics: return vk::PipelineStageFlagBits2::eAllGraphics;
			case PipelineStage::AllCommands: return vk::PipelineStageFlagBits2::eAllCommands;
			default: throw std::logic_error("Unrecognized pipeline stage!");
		}
	}

	template<>
	inline vk::PipelineStageFlags2 EnumBitsCast<vk::PipelineStageFlags2, PipelineStage>(const PipelineStage _stage)
	{
		return ConvertBitField<vk::PipelineStageFlags2, vk::PipelineStageFlagBits2, PipelineStage, 13>(_stage);
	}

	template<>
	inline vk::AccessFlagBits2 EnumCast<vk::AccessFlagBits2, Access>(const Access _access)
	{
		switch (_access)
		{
			case Access::None: return vk::AccessFlagBits2::eNone;
			case Access::IndirectCommandRead: return vk::AccessFlagBits2::eIndirectCommandRead;
			case Access::VertexBufferRead: return vk::AccessFlagBits2::eVertexAttributeRead;
			case Access::IndexBufferRead: return vk::AccessFlagBits2::eIndexRead;
			case Access::ShaderRead: return vk::AccessFlagBits2::eShaderRead;
			case Access::ShaderWrite: return vk::AccessFlagBits2::eShaderWrite;
			case Access::ColorAttachmentRead: return vk::AccessFlagBits2::eColorAttachmentRead;
			case Access::ColorAttachmentWrite: return vk::AccessFlagBits2::eColorAttachmentWrite;
			case Access::DepthStencilRead: return vk::AccessFlagBits2::eDepthStencilAttachmentRead;
			case Access::DepthStencilWrite: return vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
			case Access::TransferRead: return vk::AccessFlagBits2::eTransferRead;
			case Access::TransferWrite: return vk::AccessFlagBits2::eTransferWrite;
			default: throw std::logic_error("Unrecognized access!");
		}
	}

	template<>
	inline vk::AccessFlags2 EnumBitsCast<vk::AccessFlags2, Access>(const Access _access)
	{
		return ConvertBitField<vk::AccessFlags2, vk::AccessFlagBits2, Access, 12>(_access);
	}

	template<>
	inline vk::IndexType EnumCast<vk::IndexType, IndexType>(const IndexType _indexType)
	{
		switch (_indexType)
		{
			case IndexType::UInt8: return vk::IndexType::eUint8EXT;
			case IndexType::UInt16: return vk::IndexType::eUint16;
			case IndexType::UInt32: return vk::IndexType::eUint32;
			default: throw std::logic_error("Unrecognized index type!");
		}
	}

	template<>
	inline vk::AccessFlagBits EnumCast<vk::AccessFlagBits, Access>(const Access _access)
	{
		switch (_access)
		{
			case Access::None: return vk::AccessFlagBits::eNone;
			case Access::IndirectCommandRead: return vk::AccessFlagBits::eIndirectCommandRead;
			case Access::VertexBufferRead: return vk::AccessFlagBits::eVertexAttributeRead;
			case Access::IndexBufferRead: return vk::AccessFlagBits::eIndexRead;
			case Access::ShaderRead: return vk::AccessFlagBits::eShaderRead;
			case Access::ShaderWrite: return vk::AccessFlagBits::eShaderWrite;
			case Access::ColorAttachmentRead: return vk::AccessFlagBits::eColorAttachmentRead;
			case Access::ColorAttachmentWrite: return vk::AccessFlagBits::eColorAttachmentWrite;
			case Access::DepthStencilRead: return vk::AccessFlagBits::eDepthStencilAttachmentRead;
			case Access::DepthStencilWrite: return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			case Access::TransferRead: return vk::AccessFlagBits::eTransferRead;
			case Access::TransferWrite: return vk::AccessFlagBits::eTransferWrite;
			default: throw std::logic_error("Unrecognized access!");
		}
	}

	template<>
	inline vk::AccessFlags EnumBitsCast<vk::AccessFlags, Access>(const Access _access)
	{
		return ConvertBitField<vk::AccessFlags, vk::AccessFlagBits, Access, 12>(_access);
	}

	template<>
	inline vk::BufferUsageFlagBits EnumCast<vk::BufferUsageFlagBits, BufferUsage>(const BufferUsage _usage)
	{
		switch (_usage)
		{
			case BufferUsage::TransferSrc: return vk::BufferUsageFlagBits::eTransferSrc;
			case BufferUsage::TransferDst: return vk::BufferUsageFlagBits::eTransferDst;
			case BufferUsage::Vertex: return vk::BufferUsageFlagBits::eVertexBuffer;
			case BufferUsage::Index: return vk::BufferUsageFlagBits::eIndexBuffer;
			case BufferUsage::Uniform: return vk::BufferUsageFlagBits::eUniformBuffer;
			case BufferUsage::Storage: return vk::BufferUsageFlagBits::eStorageBuffer;
			case BufferUsage::Indirect: return vk::BufferUsageFlagBits::eIndirectBuffer;
			default: throw std::logic_error("Unrecognized buffer usage!");
		}
	}

	template<>
	inline vk::BufferUsageFlags EnumBitsCast<vk::BufferUsageFlags, BufferUsage>(const BufferUsage _usage)
	{
		return ConvertBitField<vk::BufferUsageFlags, vk::BufferUsageFlagBits, BufferUsage, 8>(_usage);
	}

	template<>
	inline vk::DescriptorType EnumCast<vk::DescriptorType, DescriptorType>(const DescriptorType _type)
	{
		switch (_type)
		{
			case DescriptorType::UniformBuffer: return vk::DescriptorType::eUniformBuffer;
			case DescriptorType::StorageBuffer: return vk::DescriptorType::eStorageBuffer;
			case DescriptorType::SampledTexture: return vk::DescriptorType::eSampledImage;
			case DescriptorType::StorageTexture: return vk::DescriptorType::eStorageImage;
			case DescriptorType::Sampler: return vk::DescriptorType::eSampler;
			case DescriptorType::CombinedImageSampler: return vk::DescriptorType::eCombinedImageSampler;
			default: throw std::logic_error("Unrecognized descriptor type!");
		}
	}

	template<>
	inline vk::ShaderStageFlagBits EnumCast<vk::ShaderStageFlagBits, ShaderStage>(const ShaderStage _stage)
	{
		switch (_stage)
		{
			case ShaderStage::Vertex: return vk::ShaderStageFlagBits::eVertex;
			case ShaderStage::Geometry: return vk::ShaderStageFlagBits::eGeometry;
			case ShaderStage::TessellationControl: return vk::ShaderStageFlagBits::eTessellationControl;
			case ShaderStage::TessellationEvaluation: return vk::ShaderStageFlagBits::eTessellationEvaluation;
			case ShaderStage::Fragment: return vk::ShaderStageFlagBits::eFragment;
			case ShaderStage::Compute: return vk::ShaderStageFlagBits::eCompute;
#if defined(VK_EXT_mesh_shader)
			case ShaderStage::Mesh: return vk::ShaderStageFlagBits::eMeshEXT;
			case ShaderStage::Task: return vk::ShaderStageFlagBits::eTaskEXT;
#endif
			default: throw std::logic_error("Unrecognized shader stage!");
		}
	}

	template<>
	inline vk::ShaderStageFlags EnumBitsCast<vk::ShaderStageFlags, ShaderStage>(const ShaderStage _stages)
	{
		vk::ShaderStageFlags result {};

		if (!Any(_stages))
		{
			return result;
		}

		if (static_cast<uint32_t>(_stages & ShaderStage::Vertex) != 0) result |= vk::ShaderStageFlagBits::eVertex;
		if (static_cast<uint32_t>(_stages & ShaderStage::Geometry) != 0) result |= vk::ShaderStageFlagBits::eGeometry;
		if (static_cast<uint32_t>(_stages & ShaderStage::TessellationControl) != 0) result |= vk::ShaderStageFlagBits::eTessellationControl;
		if (static_cast<uint32_t>(_stages & ShaderStage::TessellationEvaluation) != 0) result |= vk::ShaderStageFlagBits::eTessellationEvaluation;
		if (static_cast<uint32_t>(_stages & ShaderStage::Fragment) != 0) result |= vk::ShaderStageFlagBits::eFragment;
		if (static_cast<uint32_t>(_stages & ShaderStage::Compute) != 0) result |= vk::ShaderStageFlagBits::eCompute;
#if defined(VK_EXT_mesh_shader)
		if (static_cast<uint32_t>(_stages & ShaderStage::Mesh) != 0) result |= vk::ShaderStageFlagBits::eMeshEXT;
		if (static_cast<uint32_t>(_stages & ShaderStage::Task) != 0) result |= vk::ShaderStageFlagBits::eTaskEXT;
#endif

		return result;
	}

	template<>
	inline vk::Filter EnumCast<vk::Filter, Filter>(const Filter _filter)
	{
		switch (_filter)
		{
			case Filter::Nearest: return vk::Filter::eNearest;
			case Filter::Linear: return vk::Filter::eLinear;
			default: throw std::logic_error("Unrecognized filter!");
		}
	}

	template<>
	inline vk::SamplerMipmapMode EnumCast<vk::SamplerMipmapMode, MipmapMode>(const MipmapMode _mipmapMode)
	{
		switch (_mipmapMode)
		{
			case MipmapMode::Nearest: return vk::SamplerMipmapMode::eNearest;
			case MipmapMode::Linear: return vk::SamplerMipmapMode::eLinear;
			default: throw std::logic_error("Unrecognized mipmap mode!");
		}
	}

	template<>
	inline vk::SamplerAddressMode EnumCast<vk::SamplerAddressMode, AddressMode>(const AddressMode _addressMode)
	{
		switch (_addressMode)
		{
			case AddressMode::Repeat: return vk::SamplerAddressMode::eRepeat;
			case AddressMode::MirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
			case AddressMode::ClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
			case AddressMode::ClampToBorder: return vk::SamplerAddressMode::eClampToBorder;
			default: throw std::logic_error("Unrecognized sampler address mode!");
		}
	}
}
