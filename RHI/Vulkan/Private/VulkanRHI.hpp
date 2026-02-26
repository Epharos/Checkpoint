#pragma once

#include <RenderingHardwareInterface.hpp>

#include "Core/VulkanInstance.hpp"
#include "Core/VulkanPhysicalDevice.hpp"
#include "Core/VulkanDevice.hpp"

namespace cp
{
	class VulkanSurface;

	struct SurfaceInfo;

	class VulkanRHI final : public RenderingHardwareInterface
	{
	public:
		VulkanRHI(std::shared_ptr<cp::ILogger> _logger);
		virtual ~VulkanRHI() = default;

		IInstance& CreateInstance(const InstanceInfo& _info) override;
		IPhysicalDevice& CreatePhysicalDevice() override;
		IDevice& CreateDevice() override;

		std::unique_ptr<VulkanSurface> CreateSurface(const SurfaceInfo& _info);
		std::unique_ptr<ISwapchain> CreateSwapchain(const SwapchainInfo& _info) override;

		IInstance& GetInstance() override;
		const IInstance& GetInstance() const override;
		IPhysicalDevice& GetPhysicalDevice() override;
		const IPhysicalDevice& GetPhysicalDevice() const override;
		IDevice& GetDevice() override;
		const IDevice& GetDevice() const override;

	private:	
		std::unique_ptr<VulkanInstance> instance = nullptr;
		std::unique_ptr<VulkanPhysicalDevice> physicalDevice = nullptr;
		std::unique_ptr<VulkanDevice> device = nullptr;
	};
}