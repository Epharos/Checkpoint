#pragma once

#include <memory>

namespace cp
{
	class ILogger;
	enum class IQueueType : uint8_t;
	class IQueue;

	struct TextureInfo;
	class ITexture;

	class IDevice
	{
	public:
		IDevice(ILogger& _logger);
		virtual ~IDevice() = default;

		virtual IQueue& GetQueue(IQueueType _queueType, uint32_t _index) = 0;

		virtual std::shared_ptr<ITexture> CreateTexture(const TextureInfo& _info) = 0;

	protected:
		ILogger& logger;
	};
}