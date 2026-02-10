#pragma once

#include <string>
#include <cstdint>


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
		RHIInstance(ILogger& _logger, const RHIInstanceInfo& _info);
		virtual ~RHIInstance() = default;

	protected:
		ILogger& logger;

		RHIInstanceInfo info;
	};
}