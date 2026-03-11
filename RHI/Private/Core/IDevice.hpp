#pragma once

#include <memory>

namespace cp
{

	class ILogger;
	enum class QueueType : uint8_t;
	class IQueue;

	struct TextureInfo;
	class ITexture;
	enum class TextureLayout : uint32_t;

	class IDevice
	{
	public: // Constructors, destructor, operators
		IDevice(ILogger& _logger);
		virtual ~IDevice() = default;

	public: // Getters and Setters
		/**
		* @brief Retrieves a queue of the given type at the given index.
		*
		* @param _queueType The type of queue to retrieve (Graphics, Compute, Transfer).
		* @param _index The index of the queue within queues of that type.
		* @return The queue matching the given type and index.
		*/
		[[nodiscard]] virtual IQueue& GetQueue(QueueType _queueType, uint32_t _index) = 0;
		[[nodiscard]] virtual const IQueue& GetQueue(QueueType _queueType, uint32_t _index) const = 0;

	public: // Resource creation
		/**
		* @brief Creates a texture and allocates GPU memory for it.
		*
		* @param _info The description of the texture to create (type, format, extent, usage, ...).
		* @param _initialLayout The initial layout the texture will be considered to be in.
		* @return A shared pointer to the created texture.
		*/
		virtual std::shared_ptr<ITexture> CreateTexture(const TextureInfo& _info, TextureLayout _initialLayout) = 0;

	public: // Public methods
		/**
		* @brief Blocks the CPU until all GPU work submitted to this device has completed.
		*        Must be called before destroying any GPU resource still referenced by in-flight commands.
		*/
		virtual void WaitIdle() const = 0;

	protected:
		ILogger& logger;
	};
}