#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace cp::editorui
{
    struct KeybindSettingsEntry
    {
        std::string actionId;
        std::string displayName;
        std::string category;
        std::string defaultKey;
        std::string currentKey;
        std::string scope;
    };

    class ISettingsWindow
    {
    public:
        virtual ~ISettingsWindow() = default;

        // pair of (actionId, newKey)
        using ApplyKeybindsHandler = std::function<void(std::vector<std::pair<std::string, std::string>>)>;

        virtual void SetKeybindEntries(std::vector<KeybindSettingsEntry> _entries) = 0;
        virtual void SetApplyKeybindsHandler(ApplyKeybindsHandler _handler) = 0;

        virtual void Show() = 0;
    };
}
