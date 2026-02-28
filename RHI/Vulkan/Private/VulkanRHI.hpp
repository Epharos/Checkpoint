#pragma once

#include <RenderingHardwareInterface.hpp>

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
		VulkanRHI(std::shared_ptr<cp::ILogger> _logger);
		virtual ~VulkanRHI() = default;

		IInstance& CreateInstance(const InstanceInfo& _info) override;
		IPhysicalDevice& CreatePhysicalDevice() override;
		IDevice& CreateDevice() override;

		std::unique_ptr<Surface> CreateSurface(const SurfaceInfo& _info);
		std::unique_ptr<ISwapchain> CreateSwapchain(const SwapchainInfo& _info) override;

		IInstance& GetInstance() override;
		const IInstance& GetInstance() const override;
		IPhysicalDevice& GetPhysicalDevice() override;
		const IPhysicalDevice& GetPhysicalDevice() const override;
		IDevice& GetDevice() override;
		const IDevice& GetDevice() const override;

	private:	
		std::unique_ptr<Instance> instance = nullptr;
		std::unique_ptr<PhysicalDevice> physicalDevice = nullptr;
		std::unique_ptr<Device> device = nullptr;
	};
}