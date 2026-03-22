#include "../../Public/Resources/TextureLoader.hpp"
#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Synchro.hpp>
#include <Common/IO//FileHelper.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <cstring>

namespace cp
{
	std::shared_ptr<ITexture> AssetLoader<ITexture>::Load(
		const std::filesystem::path& path,
		RenderingHardwareInterface& rhi)
	{
		int width, height, channels;
		stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!pixels)
		{
			return nullptr;
		}

		const size_t pixelDataSize = width * height * 4;

		const BufferInfo stagingBufferInfo{
			.sizeBytes = pixelDataSize,
			.usage = BufferUsage::TransferSrc,
			.cpuVisible = true
		};
		
		auto stagingBuffer = rhi.CreateBuffer(stagingBufferInfo);

		void* mappedMemory = stagingBuffer->Map();
		if (!mappedMemory)
		{
			stbi_image_free(pixels);
			return nullptr;
		}

		std::memcpy(mappedMemory, pixels, pixelDataSize);
		stagingBuffer->Unmap();

		stbi_image_free(pixels);

		const TextureInfo textureInfo
		{
			.type = TextureType::Texture2D,
			.extent = cp::Extent3D<uint32_t> { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 },
			.mipLevels = 1,
			.arrayLayers = 1,
			.format = Format::R8G8B8A8_UNORM,
			.usage = TextureUsage::Sampled | TextureUsage::TransferDst,
			.aspect = TextureAspect::Color,
		};

		auto texture = rhi.CreateTexture(textureInfo);

		auto& graphicsQueue = rhi.GetDevice().GetQueue(QueueType::Graphics, 0);
		auto commandAllocator = rhi.CreateCommandAllocator(graphicsQueue);
		auto cmdBuffer = commandAllocator->Allocate();

		cmdBuffer->Begin();

		const TextureBarrierInfo toTransferDst{
			.texture = *texture,
			.srcLayout = TextureLayout::Undefined,
			.dstLayout = TextureLayout::TransferDst,
			.srcStage = PipelineStage::Top,
			.dstStage = PipelineStage::Transfer,
			.srcAccess = Access::None,
			.dstAccess = Access::TransferWrite,
			.baseMip = 0,
			.mipCount = 1,
			.baseLayer = 0,
			.layerCount = 1
		};
		cmdBuffer->AddBarrier(IBarrier { toTransferDst });

		const BufferTextureCopyRegion copyRegion{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
			.textureOffset = Offset3D<int32_t> { 0, 0, 0 },
			.textureExtent = Extent3D<uint32_t> { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 }
		};
		cmdBuffer->CopyBufferToTexture(*stagingBuffer, *texture, copyRegion);

		const TextureBarrierInfo toShaderReadOnly{
			.texture = *texture,
			.srcLayout = TextureLayout::TransferDst,
			.dstLayout = TextureLayout::ShaderReadOnly,
			.srcStage = PipelineStage::Transfer,
			.dstStage = PipelineStage::FragmentShader,
			.srcAccess = Access::TransferWrite,
			.dstAccess = Access::ShaderRead,
			.baseMip = 0,
			.mipCount = 1,
			.baseLayer = 0,
			.layerCount = 1
		};
		cmdBuffer->AddBarrier(IBarrier{toShaderReadOnly});

		cmdBuffer->End();

		const auto semaphore = rhi.CreateTimelineSemaphore();

		const SubmitInfo submitInfo
		{
			.commandBuffers = { cmdBuffer.get() },
			.signalInfos = {
				{ semaphore.get(), 1 }
			},
		};


		graphicsQueue.Submit(submitInfo);

		semaphore->WaitCPU(1);

		return texture;
	}
}
