#pragma once

#include <cstdint>

namespace cp
{
    enum class Key : uint16_t
    {
        Unknown = 0,

        // Letters
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // Top-row digits
        D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,

        // Function keys
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

        // Control / editing
        Space, Enter, Escape, Tab,
        Backspace, Delete, Insert,
        Home, End, PageUp, PageDown,

        // Arrow keys
        Left, Right, Up, Down,

        // Modifiers
        LeftShift, RightShift,
        LeftCtrl, RightCtrl,
        LeftAlt, RightAlt,
        LeftSuper, RightSuper,

        // Numpad
        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
        Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
        NumpadAdd, NumpadSubtract, NumpadMultiply, NumpadDivide,
        NumpadDecimal, NumpadEnter,

        // Symbols
        Minus, Equal,
        LeftBracket, RightBracket, Backslash,
        Semicolon, Apostrophe, GraveAccent,
        Comma, Period, Slash,

        // Lock / system
        CapsLock, NumLock, ScrollLock,
        PrintScreen, Pause,

        Count
    };
}
