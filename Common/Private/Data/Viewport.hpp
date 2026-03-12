#pragma once

namespace cp
{
    struct Viewport
    {
    public:
        Viewport(float _x, float _y, float _width, float _height) :
            x(_x), y(_y), width(_width), height(_height), minDepth(0), maxDepth(1)
        {}

        Viewport(float _x, float _y, float _width, float _height, float _minDepth, float _maxDepth) :
            x(_x), y(_y), width(_width), height(_height), minDepth(_minDepth), maxDepth(_maxDepth)
        {}

    public:
        [[nodiscard]] float& GetX() { return x; }
        [[nodiscard]] float& GetY() { return y; }
        [[nodiscard]] float& GetWidth() { return width; }
        [[nodiscard]] float& GetHeight() { return height; }
        [[nodiscard]] float& GetMinDepth() { return minDepth; }
        [[nodiscard]] float& GetMaxDepth() { return maxDepth; }

        [[nodiscard]] const float& GetX() const { return x; }
        [[nodiscard]] const float& GetY() const { return y; }
        [[nodiscard]] const float& GetWidth() const { return width; }
        [[nodiscard]] const float& GetHeight() const { return height; }
        [[nodiscard]] const float& GetMinDepth() const { return minDepth; }
        [[nodiscard]] const float& GetMaxDepth() const { return maxDepth; }

    private:
        float x, y;
        float width, height;
        float minDepth, maxDepth;
    };
}