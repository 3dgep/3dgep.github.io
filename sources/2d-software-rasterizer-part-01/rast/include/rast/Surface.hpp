#pragma once

#include "Color.hpp"

#include <mdspan>
#include <span>
#include <vector>

namespace rast
{

// A 2D view of pixels, indexed as pixels[y, x]: row first, then column.
using PixelView = std::mdspan<Color, std::dextents<int, 2>>;
using ConstPixelView = std::mdspan<const Color, std::dextents<int, 2>>;

// A 2D grid of pixels that lives in CPU memory.
// We draw into a Surface, and the Window shows it on screen.
class Surface
{
public:
    Surface(int width, int height);

    int Width() const { return m_Width; }
    int Height() const { return m_Height; }

    // The pixels as a 2D grid.
    PixelView Pixels() { return PixelView{ m_Pixels.data(), m_Height, m_Width }; }
    ConstPixelView Pixels() const { return ConstPixelView{ m_Pixels.data(), m_Height, m_Width }; }

    // The pixels as one long array, row after row.
    std::span<Color> Data() { return m_Pixels; }
    std::span<const Color> Data() const { return m_Pixels; }

private:
    int m_Width;
    int m_Height;
    std::vector<Color> m_Pixels;
};

} // namespace rast
