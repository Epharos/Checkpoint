add_library(Platform_GLFW SHARED
	"Private/GLFWWindow.cpp"
	"Private/PlatformGLFW.cpp"
)

target_include_directories(Platform_GLFW
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Platform_GLFW
	PUBLIC
		Common
		Platform
		glfw
)

if(TARGET Runtime_Standalone)
	add_custom_command(TARGET Platform_GLFW POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Runtime_Standalone>/Platforms"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Platform_GLFW>"
				"$<TARGET_FILE_DIR:Runtime_Standalone>/Platforms/$<TARGET_FILE_NAME:Platform_GLFW>"
		COMMENT "Copying Platform_GLFW to Runtime_Standalone/Platforms"
		VERBATIM
	)
endif()

if(TARGET Editor)
	add_custom_command(TARGET Platform_GLFW POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Editor>/Platforms"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Platform_GLFW>"
				"$<TARGET_FILE_DIR:Editor>/Platforms/$<TARGET_FILE_NAME:Platform_GLFW>"
		COMMENT "Copying Platform_GLFW to Editor/Platforms"
		VERBATIM
	)
endif()
