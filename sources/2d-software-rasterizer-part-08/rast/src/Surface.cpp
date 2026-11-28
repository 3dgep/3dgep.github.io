#include <rast/Surface.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <format>
#include <memory>

// stb_image is a single-file library: every file that includes stb_image.h gets the
// declarations, and the one file that defines STB_IMAGE_IMPLEMENTATION first gets the code.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace rast
{

Surface::Surface(int width, int height, PixelFormat format)
    : m_Width{ width }
    , m_Height{ height }
    , m_Format{ format }
    , m_Pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height))
{
    assert(width > 0 && height > 0);
}

std::expected<Surface, std::string> Surface::Load(const std::string& path)
{
    // Ask for 4 channels (red, green, blue and alpha), whatever the file has.
    // stb_image returns 4 bytes per pixel, in that order, row after row from the top.
    int width = 0;
    int height = 0;
    int channels = 0; // The number of channels in the file, which we don't need.
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> bytes{
        stbi_load(path.c_str(), &width, &height, &channels, 4), &stbi_image_free
    };
    if (!bytes)
    {
        return std::unexpected(std::format("Can't load {}: {}", path, stbi_failure_reason()));
    }

    // Our pixels are 0xAARRGGBB numbers, so we build each one from its 4 bytes.
    Surface surface{ width, height };
    const stbi_uc* rgba = bytes.get();
    for (Color& pixel : surface.m_Pixels)
    {
        pixel = Color{ rgba[0], rgba[1], rgba[2], rgba[3] };
        rgba += 4;
    }
    return surface;
}

void Surface::Clear(Color color)
{
    std::ranges::fill(m_Pixels, color);
}

} // namespace rast
