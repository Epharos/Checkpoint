#pragma once

namespace cp
{
	class ILogger;

	class RHIPhysicalDevice
	{
	public:
		RHIPhysicalDevice(ILogger& _logger);
		virtual ~RHIPhysicalDevice() = default;
		
	protected:
		ILogger& logger;
	};
}