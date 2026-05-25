#pragma once

namespace cp::ecs
{
    /**
     * Declares read-only component access for a system query.
     */
    template<typename... Component>
    struct ReadAccess
    {
    };

    /**
     * Declares writable component access for a system query.
     */
    template<typename... Component>
    struct WriteAccess
    {
    };

    /**
     * Declares components that must be absent from matched entities.
     */
    template<typename... Component>
    struct ExcludeAccess
    {
    };
}
