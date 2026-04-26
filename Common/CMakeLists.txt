add_library(Common STATIC
    "Private/Containers/SparseMap.hpp"
    "Private/Containers/SparseSet.hpp"
    "Private/Serialization/ISerializer.hpp"
    "Private/Util/Clock.cpp"
    "Private/Util/TerminalFormattingHelper.cpp"

    "Private/Log/ILogger.cpp"
    "Private/Log/Loggers/CompositeLogger.cpp"
    "Private/Log/Loggers/ConsoleLogger.cpp"
    "Private/Log/Loggers/FileLogger.cpp"
    
    "Private/Log/Message.cpp" 
    "Private/Log/Components/ComponentTypeId.hpp"
    "Private/Log/Visitors/VisitorRegistry.cpp"
    "Private/Log/Visitors/IMessageVisitor.cpp" 
    
    "Private/Log/Components/TextComponent.cpp"
    "Private/Log/Components/SurroundedComponent.cpp"

    "Private/Log/Visitors/ConsoleVisitor.cpp"
    "Private/Log/Visitors/FileVisitor.cpp"

    "Private/Profiler/ScopedProfiler.cpp"
    "Private/Profiler/ProfilerTracker.cpp"

    "Private/Data/Color.hpp"
    "Private/Data/Viewport.hpp"

    "Private/Async/JobSystem.cpp"

    "Private/Plugin/PluginHost.cpp"
    "Private/Plugin/PluginAPI.hpp"
    "Private/Plugin/PluginHost.hpp"

    "Public/Common/Serialization/ISerializer.hpp"
)

target_include_directories(Common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
