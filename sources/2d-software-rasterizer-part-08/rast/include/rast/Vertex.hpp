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
    Color color = Color::White; // White leaves a texture's colors as they are.
    Vec2 texCoord = {};         // Where the vertex is on the texture: (0, 0) top-left, (1, 1) bottom-right.
};

} // namespace rast
