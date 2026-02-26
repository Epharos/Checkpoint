add_library(RHI_Vulkan STATIC
	"Private/VulkanRHI.cpp"
	"Private/Core/VulkanInstance.cpp"
	"Private/Core/VulkanPhysicalDevice.cpp"
	"Private/Core/VulkanQueue.cpp"
	"Private/Core/VulkanDevice.cpp" 
	"Private/Core/VulkanSurface.cpp"
	"Private/Core/VulkanSwapchain.cpp"

	 
	"Private/Data/VulkanTexture.cpp" 
	"Private/Utilities/MemoryHelper.cpp"

	"Private/pch.cpp"
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