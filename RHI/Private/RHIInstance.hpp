#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "RHIPhysicalDevice.hpp"
#include "RHISurface.hpp"

namespace cp
{
	class ILogger;

	struct RHIInstanceInfo
	{
		std::string appName = "Checkpoint Application";
		uint32_t appVersion = 0x00010000; // 1.0.0
		
		uint32_t apiVersion = 0; // Use 0 to get the highest version supported by the OS

		bool enableValidationLayers = false;
	};

	class RHIInstance
	{
	public:
		RHIInstance(ILogger& _logger, const RHIInstanceInfo& _instanceInfo);
		virtual ~RHIInstance() = default;

		virtual std::unique_ptr<RHIPhysicalDevice> CreatePhysicalDevice() = 0;
		virtual std::unique_ptr<RHISurface> CreateSurface(RHISurfaceInfo _info) = 0;

	protected:
		ILogger& logger;

		RHIInstanceInfo instanceInfo;
	};
}