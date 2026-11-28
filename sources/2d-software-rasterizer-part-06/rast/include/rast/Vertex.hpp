#pragma once

#include "Color.hpp"
#include "Vec2.hpp"

namespace rast
{

// A corner of a triangle: its position, and the values (attributes) that are
// interpolated across the triangle, from one vertex to the next.
struct Vertex
{
    Vec2 position;
    Color color;
};

} // namespace rast
