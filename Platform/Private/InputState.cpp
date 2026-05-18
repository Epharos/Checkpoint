#include <Platform/Input/InputState.hpp>

namespace cp
{
    bool InputState::IsKeyJustPressed(const Key _key) const
    {
        return justPressedKeys.contains(_key);
    }

    bool InputState::IsKeyHeld(const Key _key) const
    {
        return currentKeys.contains(_key);
    }

    bool InputState::IsKeyJustReleased(const Key _key) const
    {
        return justReleasedKeys.contains(_key);
    }

    std::span<const Key> InputState::GetPressedKeys() const
    {
        return pressedKeysCache;
    }

    bool InputState::IsMouseButtonJustPressed(const MouseButton _button) const
    {
        return HasButton(justPressedMouseButtons, _button);
    }

    bool InputState::IsMouseButtonHeld(const MouseButton _button) const
    {
        return HasButton(currentMouseButtons, _button);
    }

    bool InputState::IsMouseButtonJustReleased(const MouseButton _button) const
    {
        return HasButton(justReleasedMouseButtons, _button);
    }

    glm::vec2 InputState::GetMousePosition() const { return mousePosition; }
    glm::vec2 InputState::GetMouseDelta() const { return mouseDelta; }
    glm::vec2 InputState::GetMouseScroll() const { return mouseScroll; }

    void InputState::Update(const RawInputState& _raw)
    {
        justPressedKeys.clear();
        justReleasedKeys.clear();

        const std::unordered_set<Key> newKeys(_raw.heldKeys.begin(), _raw.heldKeys.end());

        for (const Key key : newKeys)
        {
            if (!currentKeys.contains(key))
            {
                justPressedKeys.insert(key);
            }
        }

        for (const Key key : currentKeys)
        {
            if (!newKeys.contains(key))
            {
                justReleasedKeys.insert(key);
            }
        }

        currentKeys = newKeys;
        pressedKeysCache.assign(currentKeys.begin(), currentKeys.end());

        justPressedMouseButtons = _raw.mouseButtons & ~currentMouseButtons;
        justReleasedMouseButtons = ~_raw.mouseButtons & currentMouseButtons;
        currentMouseButtons = _raw.mouseButtons;

        mouseDelta = _raw.mousePosition - mousePosition;
        mousePosition = _raw.mousePosition;
        mouseScroll = _raw.mouseScroll;
    }

    bool InputState::HasButton(const uint32_t _mask, const MouseButton _button)
    {
        return (_mask & (1u << static_cast<uint32_t>(_button))) != 0;
    }
}
