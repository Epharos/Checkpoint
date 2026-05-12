#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace cp
{
    /**
     * @brief Descriptor for a single keybind action contributed by a plugin.
     */
    struct KeybindDescriptor
    {
        std::string actionId;
        std::string displayName;
        std::string category;
        std::string defaultChord;
        std::string scope = "global";
    };

    /**
     * @brief Plugin-facing interface for registering and unregistering keybinds.
     */
    class IKeybindRegistrar
    {
    public:
        virtual ~IKeybindRegistrar() = default;

        virtual void Register(KeybindDescriptor _descriptor, std::function<void()> _handler) = 0;
        virtual void Unregister(std::string_view _actionId) = 0;
    };
}
