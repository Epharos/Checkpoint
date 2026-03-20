#pragma once

#include "../pch.hpp"

#include <RHI/Rendering.hpp>

namespace cp
{
	class Device;

	class ShaderModule final : public IShaderModule
	{
	public:
		ShaderModule(const ShaderModuleInfo& _info, Device& _device);
		~ShaderModule() override;

		[[nodiscard]] ShaderBinaryFormat GetFormat() const override { return info.bytecode.format; }

		[[nodiscard]] vk::ShaderModule& GetHandle() { return shaderModule; }
		[[nodiscard]] const vk::ShaderModule& GetHandle() const { return shaderModule; }

		[[nodiscard]] const ShaderModuleInfo& GetInfo() const { return info; }

	private:
		void Initialize();
		void Cleanup() const;

	private:
		vk::ShaderModule shaderModule{ VK_NULL_HANDLE };
		Device& device;
	};
}
