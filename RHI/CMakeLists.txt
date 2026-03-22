add_library(RHI_Core STATIC

        "Private/Core/IInstance.cpp"
	"Private/Core/IPhysicalDevice.cpp"
	"Private/Core/IDevice.cpp"
	"Private/Core/ICommandBuffer.hpp"

	"Private/Data/ITexture.cpp"
	"Private/Data/IBuffer.hpp"
	"Private/Data/ISampler.hpp"
	"Private/Data/IDescriptorSetLayout.hpp"
	"Private/Data/IDescriptorSet.hpp"

	"Private/Rendering/ISwapchain.cpp"
	"Private/Rendering/ShaderStage.hpp"
	"Private/Rendering/IShaderModule.hpp"
        "Private/Rendering/PipelineStates.hpp"
	"Private/Rendering/IPipelineLayout.hpp"
	"Private/Rendering/IPipeline.hpp"

		"Private/Utilities/Converter.hpp"

	"Private/Synchro/ITimelineSemaphore.hpp"
)

target_include_directories(RHI_Core 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(RHI_Core 
	PUBLIC
		Common
)
