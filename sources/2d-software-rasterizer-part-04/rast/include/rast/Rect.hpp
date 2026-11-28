#pragma once

#include <algorithm>

namespace rast
{

// A position in pixels. x goes to the right, y goes down.
struct Point
{
    int x = 0;
    int y = 0;

    constexpr bool operator==(const Point&) const = default;
};

// A rectangle of pixels. left and top are inside the rectangle, right and
// bottom are just outside it, so the width is right - left.
struct Rect
{
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;

    // A rectangle from its top-left corner and its size.
    static constexpr Rect FromSize(int x, int y, int width, int height)
    {
        return { x, y, x + width, y + height };
    }

    constexpr int Width() const { return right - left; }
    constexpr int Height() const { return bottom - top; }

    // An empty rectangle has no pixels in it.
    constexpr bool IsEmpty() const { return right <= left || bottom <= top; }

    constexpr bool Contains(Point p) const
    {
        return p.x >= left && p.x < right && p.y >= top && p.y < bottom;
    }

    constexpr bool operator==(const Rect&) const = default;
};

// The pixels that are inside both rectangles. The result is empty when they
// don't overlap (and then its width or height can be negative).
constexpr Rect Intersect(const Rect& a, const Rect& b)
{
    return {
        std::max(a.left, b.left),
        std::max(a.top, b.top),
        std::min(a.right, b.right),
        std::min(a.bottom, b.bottom),
    };
}

static_assert(Rect::FromSize(10, 20, 30, 40) == Rect{ 10, 20, 40, 60 });
static_assert(Intersect(Rect{ 0, 0, 10, 10 }, Rect{ 5, 5, 20, 20 }) == Rect{ 5, 5, 10, 10 });
static_assert(Intersect(Rect{ 0, 0, 10, 10 }, Rect{ 10, 0, 20, 10 }).IsEmpty());

} // namespace rast
