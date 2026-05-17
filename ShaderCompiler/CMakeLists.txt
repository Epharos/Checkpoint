add_library(ShaderCompiler STATIC
    "Private/ShaderCompiler.cpp"
    "Private/ShaderCache.cpp"
    "Public/ShaderCompiler/ShaderCompiler.hpp"
    "Public/ShaderCompiler/ShaderReflection.hpp"
    "Public/ShaderCompiler/ShaderCache.hpp"
)

target_include_directories(ShaderCompiler
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Public"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Private"
)

target_link_libraries(ShaderCompiler
    PUBLIC
        Common
    PRIVATE
        slang
)
