#pragma once

#include "../pch.hpp"

#include <RHI/Rendering.hpp>

namespace cp
{
	class Device;

	class Pipeline final : public IPipeline
	{
	public:
		Pipeline(const GraphicsPipelineInfo& _info, Device& _device);
		Pipeline(const ComputePipelineInfo& _info, Device& _device);
		~Pipeline() override;

		[[nodiscard]] vk::Pipeline& GetHandle() { return pipeline; }
		[[nodiscard]] const vk::Pipeline& GetHandle() const { return pipeline; }

	private:
		void InitializeGraphics(const GraphicsPipelineInfo& _info);
		void InitializeCompute(const ComputePipelineInfo& _info);
		void Cleanup() const;

	private:
		vk::Pipeline pipeline{ VK_NULL_HANDLE };
		Device& device;
	};
}
