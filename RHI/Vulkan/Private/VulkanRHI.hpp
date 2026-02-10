#pragma once

#include <RenderingHardwareInterface.hpp>
#include <RHIInstance.hpp>

namespace cp
{
	class VulkanRHI final : public RenderingHardwareInterface
	{
	public:
		VulkanRHI(std::shared_ptr<cp::ILogger> _logger);
		virtual ~VulkanRHI() = default;

		virtual std::unique_ptr<RHIInstance> CreateInstance(const RHIInstanceInfo& _info) override;
	};
}