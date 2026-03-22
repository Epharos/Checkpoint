add_executable(TEST
	"Main.cpp"
)

target_link_libraries(TEST PRIVATE
	RHI_Vulkan
	Platform_GLFW
	Common
	Resources
)