#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "IPhysicalDevice.hpp"
#include "ISurface.hpp"

namespace cp
{
	class ILogger;

	struct InstanceInfo
	{
		std::string appName = "Checkpoint Application";
		uint32_t appVersion = 0x00010000; // 1.0.0
		
		uint32_t apiVersion = 0; // Use 0 to get the highest version supported by the OS

		bool enableValidationLayers = false;
	};

	class IInstance
	{
	public:
		IInstance(ILogger& _logger, const InstanceInfo& _instanceInfo);
		virtual ~IInstance() = default;

		virtual std::unique_ptr<IPhysicalDevice> CreatePhysicalDevice() = 0;
		virtual std::unique_ptr<ISurface> CreateSurface(SurfaceInfo _info) = 0;

	protected:
		ILogger& logger;

		InstanceInfo instanceInfo;
	};
}