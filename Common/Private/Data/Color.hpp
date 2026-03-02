#pragma once

#include <cstdint>

namespace cp
{
    struct ColorRGBA8
    {
        ColorRGBA8() : color(0, 0, 0, 0) {}
        explicit ColorRGBA8(const uint8_t _r, const uint8_t _g, const uint8_t _b, const uint8_t _a) : color(_r, _g, _b, _a) {}
        explicit ColorRGBA8(const uint32_t _color) : color(_color) {}

        uint8_t color[4];

        const uint8_t& Red() const { return color[0]; }
        const uint8_t& Green() const { return color[1]; }
        const uint8_t& Blue() const { return color[2]; }
        const uint8_t& Alpha() const { return color[3]; }

        uint8_t& Red() { return color[0]; }
        uint8_t& Green() { return color[1]; }
        uint8_t& Blue() { return color[2]; }
        uint8_t& Alpha() { return color[3]; }

        uint32_t ToUInt32() const { return color[0] | color[1] << 8 | color[2] << 16 | color[3] << 24; }
    };

    class Color
    {
    public:
        Color() : color(0, 0, 0, 0) {}
        explicit Color(const ColorRGBA8 _color) : color(_color) {}

    public:
        ColorRGBA8 ToRGBA8() const { return color; }

    private:
        ColorRGBA8 color;
    };
}
