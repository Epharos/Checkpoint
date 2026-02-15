add_library(RHI_Core STATIC
	"Private/RenderingHardwareInterface.cpp"
	"Private/RHIInstance.cpp"
	"Private/RHIPhysicalDevice.cpp"
 "Private/RHIDevice.cpp")

target_include_directories(RHI_Core 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(RHI_Core 
	PUBLIC
		Common
)