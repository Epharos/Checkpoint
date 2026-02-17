add_library(Platform_GLFW STATIC
	"Private/GLFWWindow.cpp"
)

target_include_directories(Platform_GLFW 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Platform_GLFW 
	PUBLIC
		glfw

		Common
		Platform
)