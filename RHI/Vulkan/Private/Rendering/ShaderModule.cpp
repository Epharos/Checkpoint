#include "../pch.hpp"

#include "ShaderModule.hpp"

#include "../Core/Device.hpp"

namespace cp
{
	ShaderModule::ShaderModule(const ShaderModuleInfo& _info, Device& _device)
		: IShaderModule(_info), device(_device)
	{
		Initialize();
	}

	ShaderModule::~ShaderModule()
	{
		Cleanup();
	}

	void ShaderModule::Initialize()
	{
		CP_EXPECT_MSG(info.bytecode.data, "Shader bytecode data is null");
		CP_EXPECT_MSG(info.bytecode.sizeBytes > 0, "Shader bytecode is empty");
		CP_EXPECT_MSG(info.bytecode.format == ShaderBinaryFormat::SpirV, "Vulkan backend expects SPIR-V bytecode");
		CP_EXPECT_MSG(info.bytecode.sizeBytes % 4 == 0, "SPIR-V bytecode size must be a multiple of 4");

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo;
		shaderModuleCreateInfo.setCodeSize(info.bytecode.sizeBytes);
		shaderModuleCreateInfo.setPCode(reinterpret_cast<const uint32_t*>(info.bytecode.data));

		CP_VK_CHECK(device.GetHandle().createShaderModule(&shaderModuleCreateInfo, nullptr, &shaderModule));
		CP_ENSURE_MSG(shaderModule != VK_NULL_HANDLE, "ShaderModule was not initialized");
	}

	void ShaderModule::Cleanup() const
	{
		if (shaderModule != VK_NULL_HANDLE)
		{
			device.GetHandle().destroyShaderModule(shaderModule);
		}
	}
}
