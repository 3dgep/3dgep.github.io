#pragma once

namespace rast
{

// A position (or a direction) with fractional coordinates, in pixels.
// Unlike a Point, a Vec2 can be anywhere between the pixel centers.
struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2 operator+(Vec2 other) const { return { x + other.x, y + other.y }; }
    constexpr Vec2 operator-(Vec2 other) const { return { x - other.x, y - other.y }; }
    constexpr Vec2 operator*(float scale) const { return { x * scale, y * scale }; }

    constexpr bool operator==(const Vec2&) const = default;
};

constexpr Vec2 operator*(float scale, Vec2 v)
{
    return v * scale;
}

} // namespace rast
