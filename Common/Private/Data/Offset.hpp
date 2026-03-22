#pragma once

#include "Extent.hpp"

namespace cp
{
    template<Numeric T, size_t N>
    using Offset = Extent<T, N>;

    template<Numeric T>
    using Offset2D = Extent2D<T>;

    template<Numeric T>
    using Offset3D = Extent3D<T>;
}