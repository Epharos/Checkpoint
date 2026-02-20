#pragma once

#include <RenderingHardwareInterface.hpp>
#include <IInstance.hpp>

namespace cp
{
	class VulkanRHI final : public RenderingHardwareInterface
	{
	public:
		VulkanRHI(std::shared_ptr<cp::ILogger> _logger);
		virtual ~VulkanRHI() = default;

		virtual std::unique_ptr<IInstance> CreateInstance(const InstanceInfo& _info) override;
	};
}