add_library(Builder STATIC
    "Public/Builder/UserPluginBuilder.hpp"
    "Private/UserPluginBuilder.cpp"
)

target_include_directories(Builder
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Builder
    PUBLIC
        Common
    PRIVATE
        nlohmann_json::nlohmann_json
)
