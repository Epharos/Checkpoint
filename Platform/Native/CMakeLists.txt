add_library(Platform_Native SHARED
	"Private/NativeWindow.cpp"
	"Private/PlatformNative.cpp"
)

target_include_directories(Platform_Native
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Platform_Native
	PUBLIC
		Common
		Platform
)

if(TARGET Runtime_Standalone)
	add_custom_command(TARGET Platform_Native POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Runtime_Standalone>/Platforms"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Platform_Native>"
				"$<TARGET_FILE_DIR:Runtime_Standalone>/Platforms/$<TARGET_FILE_NAME:Platform_Native>"
		COMMENT "Copying Platform_Native to Runtime_Standalone/Platforms"
		VERBATIM
	)
endif()

if(TARGET Editor)
	add_custom_command(TARGET Platform_Native POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Platform_Native>"
				"$<TARGET_FILE_DIR:Editor>/$<TARGET_FILE_NAME:Platform_Native>"
		COMMENT "Copying Platform_Native next to Editor executable"
		VERBATIM
	)
endif()
