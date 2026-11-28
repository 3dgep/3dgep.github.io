#pragma once

#include "Color.hpp"
#include "Rect.hpp"

#include <expected>
#include <mdspan>
#include <span>
#include <string>
#include <vector>

namespace rast
{

// A 2D view of pixels, indexed as pixels[x, y]. With std::layout_left, the
// left index (x) changes fastest in memory, so the pixels are stored row by row.
using PixelView = std::mdspan<Color, std::dextents<int, 2>, std::layout_left>;
using ConstPixelView = std::mdspan<const Color, std::dextents<int, 2>, std::layout_left>;

// What the bytes of the pixels mean. Both formats store 0xAARRGGBB numbers, which are the bytes
// blue, green, red, alpha in memory (on little-endian CPUs), like the DXGI formats with these names.
enum class PixelFormat
{
    B8G8R8A8Unorm,     // The bytes are blended as they are.
    B8G8R8A8UnormSrgb, // The red, green and blue bytes are sRGB: they're blended in linear light.
};

// A 2D grid of pixels that lives in CPU memory.
// We draw into a Surface, and the Window shows it on screen.
// A Surface can also be a texture: an image that we draw onto triangles.
class Surface
{
public:
    Surface(int width, int height, PixelFormat format = PixelFormat::B8G8R8A8Unorm);

    // Loads an image file (PNG, JPEG, BMP, TGA, ...) into a new surface.
    // Returns a message that says what went wrong when the file can't be loaded.
    static std::expected<Surface, std::string> Load(const std::string& path);

    int Width() const { return m_Width; }
    int Height() const { return m_Height; }
    PixelFormat Format() const { return m_Format; }

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
    PixelFormat m_Format;
    std::vector<Color> m_Pixels;
};

} // namespace rast
