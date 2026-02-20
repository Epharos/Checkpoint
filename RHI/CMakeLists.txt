add_library(RHI_Core STATIC
	"Private/RenderingHardwareInterface.cpp"
	"Private/IInstance.cpp"
	"Private/IPhysicalDevice.cpp"
	"Private/IDevice.cpp"
	"Private/ISurface.cpp")

target_include_directories(RHI_Core 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(RHI_Core 
	PUBLIC
		Common
)