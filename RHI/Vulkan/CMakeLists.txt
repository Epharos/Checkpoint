add_library(RHI_Vulkan STATIC
	"Private/VulkanRHI.cpp"
	"Private/VulkanInstance.cpp"
	"Private/VulkanPhysicalDevice.cpp"
	"Private/VulkanQueue.cpp"
 "Private/VulkanDevice.cpp")

target_include_directories(RHI_Vulkan 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(RHI_Vulkan 
	PUBLIC
		RHI_Core
		Vulkan::Vulkan
)