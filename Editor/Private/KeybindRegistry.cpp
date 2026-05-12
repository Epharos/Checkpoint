#include "KeybindRegistry.hpp"

#include <algorithm>
#include <fstream>

#include <nlohmann/json.hpp>

namespace cp::editor
{
    void KeybindRegistry::RegisterAction(KeybindEntry _entry)
    {
        if (_entry.currentChord.empty())
        {
            _entry.currentChord = _entry.defaultChord;
        }

        m_entries.push_back(std::move(_entry));
    }

    void KeybindRegistry::UnregisterAction(std::string_view _actionId)
    {
        m_entries.erase(
            std::remove_if(
                m_entries.begin(), m_entries.end(),
                [_actionId](const KeybindEntry& e) { return e.actionId == _actionId; }
                ),
            m_entries.end()
        );
    }

    std::vector<cp::editorui::KeybindSettingsEntry> KeybindRegistry::GetSettingsEntries() const
    {
        std::vector<cp::editorui::KeybindSettingsEntry> result;
        result.reserve(m_entries.size());
        for (const auto& e : m_entries)
        {
            result.push_back({
                .actionId = e.actionId,
                .displayName = e.displayName,
                .category = e.category,
                .defaultKey = e.defaultChord,
                .currentKey = e.currentChord,
                .scope = e.scope
            });
        }
        return result;
    }

    std::vector<std::pair<std::string, std::string>> KeybindRegistry::ApplyChordChanges(
        const std::vector<std::pair<std::string, std::string>>& _changes
    )
    {
        std::vector<std::pair<std::string, std::string>> applied;

        for (const auto& [actionId, chord] : _changes)
        {
            if (KeybindEntry* entry = FindEntry(actionId))
            {
                entry->currentChord = chord;
                applied.emplace_back(actionId, chord);
            }
        }

        return applied;
    }

    bool KeybindRegistry::DispatchKeyPress(std::string_view _chord, std::string_view _activeScope) const
    {
        bool consumed = false;
        if (_activeScope != KeybindScopeGlobal)
        {
            for (const auto& e : m_entries)
            {
                if (e.scope == _activeScope && e.currentChord == _chord && e.handler)
                {
                    e.handler();
                    consumed = true;
                }
            }
        }

        if (!consumed)
        {
            for (const auto& e : m_entries)
            {
                if (e.scope == KeybindScopeGlobal && e.currentChord == _chord && e.handler)
                {
                    e.handler();
                    consumed = true;
                }
            }
        }

        return consumed;
    }

    void KeybindRegistry::SaveToFile(const std::filesystem::path& _path) const
    {
        nlohmann::json root = nlohmann::json::object();

        for (const auto& e : m_entries)
        {
            if (e.currentChord != e.defaultChord)
            {
                root[e.actionId] = e.currentChord;
            }
        }

        std::ofstream file(_path);
        file << root.dump(4);
    }

    void KeybindRegistry::LoadFromFile(const std::filesystem::path& _path)
    {
        std::ifstream file(_path);

        if (!file.is_open())
        {
            return;
        }

        try
        {
            const nlohmann::json root = nlohmann::json::parse(file);

            for (const auto& [actionId, chord] : root.items())
            {
                if (KeybindEntry* entry = FindEntry(actionId))
                {
                    entry->currentChord = chord.get<std::string>();
                }
            }
        }
        catch (...) {}
    }

    KeybindEntry* KeybindRegistry::FindEntry(std::string_view _actionId)
    {
        for (auto& e : m_entries)
        {
            if (e.actionId == _actionId)
            {
                return &e;
            }
        }

        return nullptr;
    }

    const KeybindEntry* KeybindRegistry::FindEntry(std::string_view _actionId) const
    {
        for (const auto& e : m_entries)
        {
            if (e.actionId == _actionId)
            {
                return &e;
            }
        }

        return nullptr;
    }
}
