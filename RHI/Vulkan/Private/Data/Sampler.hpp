#pragma once

#include "../pch.hpp"

#include <RHI/Data.hpp>

namespace cp
{
	class Device;

	class Sampler final : public ISampler
	{
	public:
		Sampler(const SamplerInfo& _info, Device& _device);
		~Sampler() override;

		[[nodiscard]] vk::Sampler& GetHandle() { return sampler; }
		[[nodiscard]] const vk::Sampler& GetHandle() const { return sampler; }

	private:
		void Initialize();
		void Cleanup() const;

	private:
		vk::Sampler sampler{ VK_NULL_HANDLE };
		Device& device;
	};
}
