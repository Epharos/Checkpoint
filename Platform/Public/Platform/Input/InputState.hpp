#pragma once

#include "Key.hpp"
#include "MouseButton.hpp"
#include "RawInputState.hpp"

#include <glm/vec2.hpp>

#include <cstdint>
#include <span>
#include <unordered_set>
#include <vector>

namespace cp
{
    class InputState
    {
    public:
        /** @brief True only on the first frame the key is pressed. */
        [[nodiscard]] bool IsKeyJustPressed(Key _key) const;

        /** @brief True every frame while the key is held down. */
        [[nodiscard]] bool IsKeyHeld(Key _key) const;

        /** @brief True only on the first frame the key is released. */
        [[nodiscard]] bool IsKeyJustReleased(Key _key) const;

        /** @brief All keys currently held down. */
        [[nodiscard]] std::span<const Key> GetPressedKeys() const;

        [[nodiscard]] bool IsMouseButtonJustPressed(MouseButton _button) const;
        [[nodiscard]] bool IsMouseButtonHeld(MouseButton _button) const;
        [[nodiscard]] bool IsMouseButtonJustReleased(MouseButton _button) const;

        /** @brief Absolute cursor position in client-area pixels. */
        [[nodiscard]] glm::vec2 GetMousePosition() const;

        /** @brief Cursor movement since the previous frame. */
        [[nodiscard]] glm::vec2 GetMouseDelta() const;

        /**
         * @brief Scroll wheel delta this frame.
         * x = horizontal scroll, y = vertical scroll.
         */
        [[nodiscard]] glm::vec2 GetMouseScroll() const;

        /** @brief Called once per frame by the runtime after PollEvents(). */
        void Update(const RawInputState& _raw);

    private:
        static bool HasButton(uint32_t _mask, MouseButton _button);

        std::unordered_set<Key> currentKeys;
        std::unordered_set<Key> justPressedKeys;
        std::unordered_set<Key> justReleasedKeys;
        std::vector<Key> pressedKeysCache;

        uint32_t currentMouseButtons = 0;
        uint32_t justPressedMouseButtons = 0;
        uint32_t justReleasedMouseButtons = 0;

        glm::vec2 mousePosition { 0.f, 0.f };
        glm::vec2 mouseDelta { 0.f, 0.f };
        glm::vec2 mouseScroll { 0.f, 0.f };
    };
}
