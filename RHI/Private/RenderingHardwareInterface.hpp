#pragma once

#include <../../Common/Public/Common/Core/Log.hpp>
#include "Rendering/IShaderModule.hpp"

namespace cp
{
	class IInstance;
	class IPhysicalDevice;
	class IDevice;
	class IQueue;
	class ISwapchain;
	class ICommandAllocator;
	class ITimelineSemaphore;
	class ITexture;
	class IBuffer;
	class ISampler;

	struct TextureInfo;
	struct BufferInfo;
	struct SamplerInfo;
	struct InstanceInfo;
	struct SwapchainInfo;

	enum class TextureLayout : uint32_t;

	class RenderingHardwareInterface
	{
	public:
		explicit RenderingHardwareInterface(const std::shared_ptr<ILogger> &_logger)
			: logger(_logger) {}

		virtual ~RenderingHardwareInterface() = default;

		virtual IInstance& CreateInstance(const InstanceInfo& _info) = 0;
		virtual IPhysicalDevice& CreatePhysicalDevice() = 0;
		virtual IDevice& CreateDevice() = 0;

		virtual std::unique_ptr<ISwapchain> CreateSwapchain(const SwapchainInfo& _info) = 0;

		virtual std::shared_ptr<ITexture> CreateTexture(const TextureInfo& _info) = 0;
		virtual std::shared_ptr<IBuffer> CreateBuffer(const BufferInfo& _info) = 0;
		virtual std::shared_ptr<ISampler> CreateSampler(const SamplerInfo& _info) = 0;

		virtual std::unique_ptr<ITimelineSemaphore> CreateTimelineSemaphore() = 0;

		virtual std::unique_ptr<ICommandAllocator> CreateCommandAllocator(IQueue& _queue) = 0;

		[[nodiscard]] virtual IInstance& GetInstance() = 0;
		[[nodiscard]] virtual const IInstance& GetInstance() const = 0;
		[[nodiscard]] virtual IPhysicalDevice& GetPhysicalDevice() = 0;
		[[nodiscard]] virtual const IPhysicalDevice& GetPhysicalDevice() const = 0;
		[[nodiscard]] virtual IDevice& GetDevice() = 0;
		[[nodiscard]] virtual const IDevice& GetDevice() const = 0;

		/** @brief Returns the binary shader format produced by this RHI backend. */
		[[nodiscard]] virtual ShaderBinaryFormat GetShaderBinaryFormat() const = 0;

		static constexpr const char* RHI_Label = "RHI";

	protected:
		[[nodiscard]] ILogger& GetLogger() const { return *logger; }

	private:
		std::shared_ptr<ILogger> logger;
	};

	using RHI = RenderingHardwareInterface;
}