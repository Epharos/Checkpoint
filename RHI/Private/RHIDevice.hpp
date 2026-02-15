#pragma once

#include <memory>

namespace cp
{
	class ILogger;
	enum class RHIQueueType : uint8_t;
	class RHIQueue;

	class RHIDevice
	{
	public:
		RHIDevice(ILogger& _logger);
		virtual ~RHIDevice() = default;

		virtual RHIQueue& GetQueue(RHIQueueType _queueType, uint32_t _index) = 0;

	protected:
		ILogger& logger;
	};
}