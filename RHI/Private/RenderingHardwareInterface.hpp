#pragma once

#include <Log.hpp>

namespace cp
{
	class RHIInstance;
	struct RHIInstanceInfo;

	class RenderingHardwareInterface
	{
	public:
		std::shared_ptr<cp::ILogger> logger;

		RenderingHardwareInterface(std::shared_ptr<cp::ILogger> _logger);
		virtual ~RenderingHardwareInterface() = default;

		virtual std::unique_ptr<RHIInstance> CreateInstance(const RHIInstanceInfo& _info) = 0;

	protected:
		ILogger& GetLogger() const;
	};
}