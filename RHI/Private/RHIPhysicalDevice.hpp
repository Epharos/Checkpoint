#pragma once

#include <memory> 

#include "RHIDevice.hpp"

namespace cp
{
	class ILogger;

	class RHIPhysicalDevice
	{
	public:
		RHIPhysicalDevice(ILogger& _logger);
		virtual ~RHIPhysicalDevice() = default;

		virtual std::unique_ptr<RHIDevice> CreateDevice() = 0;
		
	protected:
		ILogger& logger;
	};
}