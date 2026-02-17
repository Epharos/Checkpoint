add_library(Platform STATIC
	"Private/IWindow.cpp"
)

target_include_directories(Platform 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Platform 
	PUBLIC
		Common
)