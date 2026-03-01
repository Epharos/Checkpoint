#pragma once

#include <memory>

namespace cp
{
	class ILogger;
	enum class QueueType : uint8_t;
	class IQueue;

	struct TextureInfo;
	class ITexture;

	class IDevice
	{
	public: // Constructors, destructor, operators
		IDevice(ILogger& _logger);
		virtual ~IDevice() = default;

	public: // Getters and Setters
		virtual IQueue& GetQueue(QueueType _queueType, uint32_t _index) = 0;

	public: // Resource creation
		virtual std::shared_ptr<ITexture> CreateTexture(const TextureInfo& _info) = 0;

	public: // Public methods
		virtual void WaitIdle() const = 0;

	protected:
		ILogger& logger;
	};
}