#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "ShaderStage.hpp"

namespace cp
{
	enum class ShaderBinaryFormat : uint8_t
	{
		SpirV,
		Dxil,
	};

	struct ShaderBytecode
	{
		ShaderBinaryFormat format = ShaderBinaryFormat::SpirV;

		const uint8_t* data = nullptr; // NOTE : Maybe std::byte instead ?
		size_t sizeBytes = 0;
	};

	struct ShaderModuleInfo
	{
		ShaderStage stages = ShaderStage::None;
		ShaderBytecode bytecode;
	};

	class IShaderModule
	{
	public:
		explicit IShaderModule(const ShaderModuleInfo& _info) : info(_info) {}
		virtual ~IShaderModule() = default;

		[[nodiscard]] virtual ShaderBinaryFormat GetFormat() const = 0;

	protected:
		const ShaderModuleInfo info;
	};
}
