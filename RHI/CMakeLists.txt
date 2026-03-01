add_library(RHI_Core STATIC
	"Private/RenderingHardwareInterface.cpp"
	"Private/Core/IInstance.cpp"
	"Private/Core/IPhysicalDevice.cpp"
	"Private/Core/IDevice.cpp"
	 
	"Private/Rendering/ISwapchain.cpp"

	"Private/Data/ITexture.cpp"

	"Private/Utilities/Converter.hpp"
		Private/Synchro/ITimelineSemaphore.hpp
		Public/RHI/Rendering.hpp
		Private/Core/ICommandBuffer.hpp
)

target_include_directories(RHI_Core 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(RHI_Core 
	PUBLIC
		Common
)