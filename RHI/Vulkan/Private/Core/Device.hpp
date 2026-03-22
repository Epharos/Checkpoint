#pragma once

#include "../pch.hpp"

#include <RHI/Core.hpp>
#include <RHI/Data.hpp>

#include <vector>
#include <array>

#include "Queue.hpp"

namespace cp
{
	class ILogger;
	class Instance;
	class PhysicalDevice;

	class Device final : public IDevice
	{
	public: // Constructors, destructor, operators
		Device(ILogger& _logger, PhysicalDevice& _physicalDevice);
		~Device() override;

	public: // Getters and Setters
		[[nodiscard]] IQueue& GetQueue(QueueType _queueType, uint32_t _index) override;
		[[nodiscard]] const IQueue& GetQueue(QueueType _queueType, uint32_t _index) const override;

		/**
		* @brief Returns the command pool associated with a queue of the given type at the given index.
		*        Used internally when allocating command buffers.
		*
		* @param _queueType The type of queue whose command pool to retrieve.
		* @param _index The index of the queue within that type.
		* @return The command pool for that queue.
		*/
		[[nodiscard]] vk::CommandPool& GetCommandPool(QueueType _queueType, uint32_t _index);
		[[nodiscard]] const vk::CommandPool& GetCommandPool(QueueType _queueType, uint32_t _index) const;

		[[nodiscard]] vk::Device& GetHandle() { return device; }
		[[nodiscard]] const vk::Device& GetHandle() const { return device; }

		[[nodiscard]] PhysicalDevice& GetPhysicalDevice() { return physicalDevice; }
		[[nodiscard]] const PhysicalDevice& GetPhysicalDevice() const { return physicalDevice; }

		/**
		* @brief Returns the queue family indices selected for this device.
		*        Used when creating resources that depend on a specific queue family (e.g. swapchain, command pools).
		*
		* @return The selected Vulkan queue family indices.
		*/
		[[nodiscard]] VulkanQueueFamilies& GetQueueFamilies() { return families; }
		[[nodiscard]] const VulkanQueueFamilies& GetQueueFamilies() const { return families; }

	public: // Resource creation
		std::shared_ptr<ITexture> CreateTexture(const TextureInfo& _info) override;
		std::shared_ptr<IBuffer> CreateBuffer(const BufferInfo& _info) override;

		std::shared_ptr<IShaderModule> CreateShaderModule(const ShaderModuleInfo& _info) override;
		std::shared_ptr<IDescriptorSetLayout> CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& _info) override;
		std::shared_ptr<IPipelineLayout> CreatePipelineLayout(const PipelineLayoutInfo& _info) override;
		std::shared_ptr<IDescriptorSet> CreateDescriptorSet(const IDescriptorSetLayout& _descriptorSetLayout) override;

		std::shared_ptr<IPipeline> CreateGraphicsPipeline(const GraphicsPipelineInfo& _info) override;
		std::shared_ptr<IPipeline> CreateComputePipeline(const ComputePipelineInfo& _info) override;

	private: // Initialization and cleanup
		void Initialize();

		void CleanupCommandPools() const;

		void Cleanup() const;

		vk::PhysicalDeviceDynamicRenderingFeatures FillFeatureStructures();

		bool CreateLogicalDevice();
		void CreateQueues();
		void CreateCommandPools();

	public: // Override methods
		void WaitIdle() const override;

	private:
		vk::Device device{ VK_NULL_HANDLE };

		VulkanQueueFamilies families;

		// 0 : Graphics, 1 : Compute, 2 : Transfer
		std::array<std::vector<std::unique_ptr<Queue>>, 3> queues;
		std::array<std::vector<vk::CommandPool>, 3> commandPools;

		PhysicalDevice& physicalDevice;
	};
}