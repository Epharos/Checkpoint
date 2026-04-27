add_library(Resources STATIC
	"Private/AssetRegistry.cpp"
	"Private/Loaders/TextureLoader.cpp"
	"Private/Loaders/MeshLoader.cpp"
)

target_include_directories(Resources 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
	PRIVATE
		${STB_INCLUDE_DIR}
)

target_link_libraries(Resources 
	PUBLIC
		Common
		RHI_Core
	PRIVATE
		assimp::assimp
)
