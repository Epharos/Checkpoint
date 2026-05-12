#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <EditorUI/SettingsWindow.hpp>

namespace cp::editor
{
    inline constexpr std::string_view KeybindScopeGlobal = "global";
    inline constexpr std::string_view KeybindScopeViewport = "viewport";

    struct KeybindEntry
    {
        std::string actionId;
        std::string displayName;
        std::string category;
        std::string defaultChord;
        std::string currentChord;
        std::string scope = std::string(KeybindScopeGlobal);
        std::function<void()> handler;
    };

    class KeybindRegistry
    {
    public:
        void RegisterAction(KeybindEntry _entry);
        void UnregisterAction(std::string_view _actionId);

        [[nodiscard]] std::vector<cp::editorui::KeybindSettingsEntry> GetSettingsEntries() const;

        std::vector<std::pair<std::string, std::string>> ApplyChordChanges(
            const std::vector<std::pair<std::string, std::string>>& _changes
        );

        [[nodiscard]] bool DispatchKeyPress(std::string_view _chord, std::string_view _activeScope) const;

        [[nodiscard]] const std::vector<KeybindEntry>& GetEntries() const { return m_entries; }

        void SaveToFile(const std::filesystem::path& _path) const;
        void LoadFromFile(const std::filesystem::path& _path);

    private:
        KeybindEntry* FindEntry(std::string_view _actionId);
        [[nodiscard]] const KeybindEntry* FindEntry(std::string_view _actionId) const;

        std::vector<KeybindEntry> m_entries;
    };
}
