#pragma once

#include <RHI/RenderingHardwareInterface.hpp>

#include "Core/Instance.hpp"
#include "Core/PhysicalDevice.hpp"
#include "Core/Device.hpp"

namespace cp
{
	class Surface;

	struct SurfaceInfo;

	class VulkanRHI final : public RenderingHardwareInterface
	{
	public:
		VulkanRHI(const std::shared_ptr<cp::ILogger> &_logger);
		~VulkanRHI() override = default;

		IInstance& CreateInstance(const InstanceInfo& _info) override;
		IPhysicalDevice& CreatePhysicalDevice() override;
		IDevice& CreateDevice() override;

		[[nodiscard]] std::unique_ptr<Surface> CreateSurface(const SurfaceInfo& _info) const;
		std::unique_ptr<ISwapchain> CreateSwapchain(const SwapchainInfo& _info) override;

		[[nodiscard]] IInstance& GetInstance() override;
		[[nodiscard]] const IInstance& GetInstance() const override;
		[[nodiscard]] IPhysicalDevice& GetPhysicalDevice() override;
		[[nodiscard]] const IPhysicalDevice& GetPhysicalDevice() const override;
		[[nodiscard]] IDevice& GetDevice() override;
		[[nodiscard]] const IDevice& GetDevice() const override;

		std::shared_ptr<ITexture> CreateTexture(const TextureInfo &_info, TextureLayout _initialLayout) override;

		std::unique_ptr<ITimelineSemaphore> CreateTimelineSemaphore() override;

		std::unique_ptr<ICommandAllocator> CreateCommandAllocator(IQueue& _queue) override;

	private:	
		std::unique_ptr<Instance> instance = nullptr;
		std::unique_ptr<PhysicalDevice> physicalDevice = nullptr;
		std::unique_ptr<Device> device = nullptr;
	};
}