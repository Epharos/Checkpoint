#pragma once

#include <memory> 

namespace cp
{
	class ILogger;

	class IPhysicalDevice
	{
	public:
		IPhysicalDevice(ILogger& _logger);
		virtual ~IPhysicalDevice() = default;

	protected:
		ILogger& logger;
	};
}