#pragma once

#include "Extent.hpp"

namespace cp
{
    struct Rectangle2D
    {
        Extent2D<int> offset;
        Extent2D<uint32_t> extent;

        Rectangle2D(const Extent2D<int> _offset, const Extent2D<uint32_t> _extent) :
            offset(_offset), extent(_extent) {}
    };

    struct Rectangle3D
    {
        Extent3D<int> offset;
        Extent3D<uint32_t> extent;

        Rectangle3D(const Extent3D<int> _offset, const Extent3D<uint32_t> _extent) :
            offset(_offset), extent(_extent) {}
    };
}