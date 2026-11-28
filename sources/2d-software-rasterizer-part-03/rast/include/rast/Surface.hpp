#pragma once

#include "Color.hpp"
#include "Rect.hpp"

#include <mdspan>
#include <span>
#include <vector>

namespace rast
{

// A 2D view of pixels, indexed as pixels[x, y]. With std::layout_left, the
// left index (x) changes fastest in memory, so the pixels are stored row by row.
using PixelView = std::mdspan<Color, std::dextents<int, 2>, std::layout_left>;
using ConstPixelView = std::mdspan<const Color, std::dextents<int, 2>, std::layout_left>;

// A 2D grid of pixels that lives in CPU memory.
// We draw into a Surface, and the Window shows it on screen.
class Surface
{
public:
    Surface(int width, int height);

    int Width() const { return m_Width; }
    int Height() const { return m_Height; }

    // The rectangle that covers every pixel of the surface.
    Rect Bounds() const { return { 0, 0, m_Width, m_Height }; }

    // Sets every pixel of the surface to the same color.
    void Clear(Color color);

    // The pixels as a 2D grid.
    PixelView Pixels() { return PixelView{ m_Pixels.data(), m_Width, m_Height }; }
    ConstPixelView Pixels() const { return ConstPixelView{ m_Pixels.data(), m_Width, m_Height }; }

    // The pixels as one long array, row after row.
    std::span<Color> Data() { return m_Pixels; }
    std::span<const Color> Data() const { return m_Pixels; }

private:
    int m_Width;
    int m_Height;
    std::vector<Color> m_Pixels;
};

} // namespace rast
