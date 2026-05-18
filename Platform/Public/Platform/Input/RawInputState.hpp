#pragma once

#include "Key.hpp"
#include "MouseButton.hpp"

#include <glm/vec2.hpp>

#include <cstdint>
#include <vector>

namespace cp
{
    /**
     * Raw input snapshot produced by the platform window each frame.
     * Filled by IWindow::FillRawInputState() after PollEvents().
     * The scroll field is accumulated between two calls and reset after each fill.
     */
    struct RawInputState
    {
        std::vector<Key> heldKeys;

        glm::vec2 mousePosition { 0.f, 0.f };
        glm::vec2 mouseScroll { 0.f, 0.f }; // x = horizontal, y = vertical
        uint32_t mouseButtons = 0; // bitmask: 1 << static_cast<uint32_t>(MouseButton)
    };
}
