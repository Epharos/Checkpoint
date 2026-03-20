#pragma once

#include <cstdint>
#include <memory>

namespace cp
{

	class ILogger;
	enum class QueueType : uint8_t;
	class IQueue;

	struct TextureInfo;
	class ITexture;
	enum class TextureLayout : uint32_t;

	// Pipeline / Shaders
	struct ShaderModuleInfo;
	class IShaderModule;

	struct DescriptorSetLayoutInfo;
	class IDescriptorSetLayout;

	struct PipelineLayoutInfo;
	class IPipelineLayout;

	struct GraphicsPipelineInfo;
	struct ComputePipelineInfo;
	class IPipeline;

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

		/**
		 * @brief Creates a shader module from backend-specific bytecode (SPIR-V, DXIL, ...).
		 */
		virtual std::shared_ptr<IShaderModule> CreateShaderModule(const ShaderModuleInfo& _info) = 0;

		/**
		 * @brief Creates a descriptor set layout (Vulkan) / root parameter table description (DX12).
		 */
		virtual std::shared_ptr<IDescriptorSetLayout> CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& _info) = 0;

		/**
		 * @brief Creates a pipeline layout (Vulkan VkPipelineLayout / DX12 RootSignature).
		 */
		virtual std::shared_ptr<IPipelineLayout> CreatePipelineLayout(const PipelineLayoutInfo& _info) = 0;

		virtual std::shared_ptr<IPipeline> CreateGraphicsPipeline(const GraphicsPipelineInfo& _info) = 0;
		virtual std::shared_ptr<IPipeline> CreateComputePipeline(const ComputePipelineInfo& _info) = 0;

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