#pragma once

#include "../pch.hpp"

#include <RHI/Rendering.hpp>

namespace cp
{
	class Device;

	class PipelineLayout final : public IPipelineLayout
	{
	public:
		PipelineLayout(const PipelineLayoutInfo& _info, Device& _device);
		~PipelineLayout() override;

		[[nodiscard]] vk::PipelineLayout& GetHandle() { return pipelineLayout; }
		[[nodiscard]] const vk::PipelineLayout& GetHandle() const { return pipelineLayout; }

	private:
		void Initialize();
		void Cleanup() const;

	private:
		vk::PipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		Device& device;
	};
}
