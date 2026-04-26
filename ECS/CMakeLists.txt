add_library(ECS STATIC
    "Private/Archetype.cpp"
    "Private/Archetype.hpp"
    "Private/ComponentRegistration.hpp"
    "Private/ComponentTypeInfo.hpp"
    "Private/System.hpp"
    "Private/TypeGuid.hpp"
    "Private/World.cpp"
    "Private/World.hpp"
    "Private/Entity.hpp"
    "Private/Access.hpp"
    "Private/CommandBuffer.hpp"

    "Public/ECS/ECS.hpp"
    "Public/ECS/World.hpp"
    "Public/ECS/Entity.hpp"
    "Public/ECS/Access.hpp"
    "Public/ECS/ComponentRegistration.hpp"
    "Public/ECS/CommandBuffer.hpp"
    "Public/ECS/System.hpp"
    "Public/ECS/TypeGuid.hpp"
)

target_include_directories(ECS
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(ECS
    PUBLIC
        Common
)
