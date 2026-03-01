#pragma once

#include <Log.hpp>

namespace cp
{
	class ITimelineSemaphore;
	struct TextureInfo;
	class ITexture;
	class IInstance;
	class IPhysicalDevice;
	class IDevice;

	class ISwapchain;

	struct InstanceInfo;
	struct SwapchainInfo;
	
	class RenderingHardwareInterface
	{
	public:
		RenderingHardwareInterface(const std::shared_ptr<ILogger> &_logger);
		virtual ~RenderingHardwareInterface() = default;

		virtual IInstance& CreateInstance(const InstanceInfo& _info) = 0;
		virtual IPhysicalDevice& CreatePhysicalDevice() = 0;
		virtual IDevice& CreateDevice() = 0;

		virtual std::unique_ptr<ISwapchain> CreateSwapchain(const SwapchainInfo& _info) = 0;

		virtual std::shared_ptr<ITexture> CreateTexture(const TextureInfo& _info) = 0;

		virtual std::shared_ptr<ITimelineSemaphore> CreateTimelineSemaphore() = 0;

		virtual IInstance& GetInstance() = 0;
		virtual const IInstance& GetInstance() const = 0;
		virtual IPhysicalDevice& GetPhysicalDevice() = 0;
		virtual const IPhysicalDevice& GetPhysicalDevice() const = 0;
		virtual IDevice& GetDevice() = 0;
		virtual const IDevice& GetDevice() const = 0;

		static constexpr const char* RHI_Label = "RHI";

	protected:
		ILogger& GetLogger() const;

	private:
		std::shared_ptr<ILogger> logger;
	};

	using RHI = RenderingHardwareInterface;
}