#pragma once

#include <memory> 

#include "IDevice.hpp"

namespace cp
{
	class ILogger;

	class IPhysicalDevice
	{
	public:
		IPhysicalDevice(ILogger& _logger);
		virtual ~IPhysicalDevice() = default;

		virtual std::unique_ptr<IDevice> CreateDevice() = 0;
		
	protected:
		ILogger& logger;
	};
}