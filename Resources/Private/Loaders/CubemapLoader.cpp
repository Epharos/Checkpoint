#include "../../Public/Resources/CubemapLoader.hpp"

#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Synchro.hpp>
#include <RHI/Data.hpp>

#include <stb_image.h>

#include <array>
#include <cstring>

namespace cp
{
	namespace
	{
		constexpr std::array<const char*, 6> FaceSuffixes = {
			"_px.jpg", "_nx.jpg",
			"_py.jpg", "_ny.jpg",
			"_pz.jpg", "_nz.jpg"
		};
	}

	std::shared_ptr<ITexture> LoadCubemapTexture(
		const std::filesystem::path& basePath,
		RenderingHardwareInterface& rhi
	)
	{
		const std::string baseStr = basePath.string();

		struct FaceData
		{
			stbi_uc* pixels = nullptr;
			int width = 0;
			int height = 0;
		};

		std::array<FaceData, 6> faces{};
		int faceWidth = 0;
		int faceHeight = 0;

		for (size_t i = 0; i < 6; ++i)
		{
			const std::string facePath = baseStr + FaceSuffixes[i];
			int channels = 0;
			faces[i].pixels = stbi_load(facePath.c_str(), &faces[i].width, &faces[i].height, &channels, STBI_rgb_alpha);

			if (!faces[i].pixels)
			{
				for (size_t j = 0; j < i; ++j)
					stbi_image_free(faces[j].pixels);

				return nullptr;
			}

			if (i == 0)
			{
				faceWidth  = faces[i].width;
				faceHeight = faces[i].height;
			}
			else if (faces[i].width != faceWidth || faces[i].height != faceHeight)
			{
				for (size_t j = 0; j <= i; ++j)
					stbi_image_free(faces[j].pixels);

				return nullptr;
			}
		}

		const size_t faceBytes = static_cast<size_t>(faceWidth) * static_cast<size_t>(faceHeight) * 4;
		const size_t totalBytes = faceBytes * 6;

		const BufferInfo stagingBufferInfo {
			.sizeBytes = totalBytes,
			.usage = BufferUsage::TransferSrc,
			.cpuVisible = true
		};

		const auto stagingBuffer = rhi.CreateBuffer(stagingBufferInfo);

		if (!stagingBuffer)
		{
			for (auto& f : faces)
				stbi_image_free(f.pixels);

			return nullptr;
		}

		void* mapped = stagingBuffer->Map();

		if (!mapped)
		{
			for (auto& f : faces)
				stbi_image_free(f.pixels);

			return nullptr;
		}

		for (size_t i = 0; i < 6; ++i)
		{
			std::memcpy(static_cast<uint8_t*>(mapped) + i * faceBytes, faces[i].pixels, faceBytes);
			stbi_image_free(faces[i].pixels);
		}

		stagingBuffer->Unmap();

		const TextureInfo textureInfo {
			.type = TextureType::TextureCube,
			.extent = Extent3D<uint32_t>{ static_cast<uint32_t>(faceWidth), static_cast<uint32_t>(faceHeight), 1 },
			.mipLevels = 1,
			.arrayLayers = 6,
			.format = Format::R8G8B8A8_UNORM,
			.usage = TextureUsage::Sampled | TextureUsage::TransferDst,
			.aspect = TextureAspect::Color,
		};

		auto texture = rhi.CreateTexture(textureInfo);

		if (!texture)
			return nullptr;

		auto& graphicsQueue = rhi.GetDevice().GetQueue(QueueType::Graphics, 0);
		const auto cmdAllocator = rhi.CreateCommandAllocator(graphicsQueue);
		const auto cmdBuffer = cmdAllocator->Allocate();

		cmdBuffer->Begin();

		const TextureBarrierInfo toTransferDst {
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
			.layerCount = 6
		};

		cmdBuffer->AddBarrier(toTransferDst);

		for (uint32_t face = 0; face < 6; ++face)
		{
			const BufferTextureCopyRegion copyRegion {
				.bufferOffset = face * faceBytes,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.mipLevel = 0,
				.baseArrayLayer = face,
				.layerCount = 1,
				.textureOffset = Offset3D<int32_t>{ 0, 0, 0 },
				.textureExtent = Extent3D<uint32_t>{ static_cast<uint32_t>(faceWidth), static_cast<uint32_t>(faceHeight), 1 }
			};

			cmdBuffer->CopyBufferToTexture(*stagingBuffer, *texture, copyRegion);
		}

		const TextureBarrierInfo toShaderReadOnly {
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
			.layerCount = 6
		};

		cmdBuffer->AddBarrier(toShaderReadOnly);

		cmdBuffer->End();

		const auto semaphore = rhi.CreateTimelineSemaphore();
		const SubmitInfo submitInfo {
			.commandBuffers = { cmdBuffer.get() },
			.signalInfos = { { semaphore.get(), 1 } },
		};

		graphicsQueue.Submit(submitInfo);
		semaphore->WaitCPU(1);

		return texture;
	}
}
