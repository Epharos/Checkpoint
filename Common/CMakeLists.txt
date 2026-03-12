add_library(Common STATIC
    "Private/Assert.hpp"

    "Private/Utilities/Clock.cpp"
    "Private/Utilities/TerminalFormattingHelper.cpp"

    "Private/Log/ILogger.cpp"
    "Private/Log/CompositeLogger.cpp"
    "Private/Log/ConsoleLogger.cpp" 
    "Private/Log/FileLogger.cpp"
    
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
        Private/Data/Color.hpp
        Private/Data/Viewport.hpp
)

target_include_directories(Common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
)