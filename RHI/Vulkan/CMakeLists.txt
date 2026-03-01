add_library(RHI_Vulkan STATIC
	"Private/VulkanRHI.cpp"
	"Private/Core/Instance.cpp"
	"Private/Core/PhysicalDevice.cpp"
	"Private/Core/Queue.cpp"
	"Private/Core/Device.cpp"
	"Private/Rendering/Surface.cpp"
	"Private/Rendering/Swapchain.cpp"

	 
	"Private/Data/Texture.cpp"
	"Private/Utilities/MemoryHelper.cpp"

	"Private/pch.cpp"
		Private/Synchro/TimelineSemaphore.cpp
		Private/Synchro/TimelineSemaphore.hpp
        Private/Core/CommandBuffer.cpp
        Private/Core/CommandBuffer.hpp
)

target_include_directories(RHI_Vulkan 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(RHI_Vulkan 
	PUBLIC
		RHI_Core
		Vulkan::Vulkan
)

target_precompile_headers(RHI_Vulkan 
	PRIVATE 
		"Private/pch.hpp")