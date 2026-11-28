#pragma once

#include <cstdint>

namespace rast
{

// A 32-bit color, packed as 0xAARRGGBB (SDL_PIXELFORMAT_ARGB8888).
//
//   bits 31..24  alpha
//   bits 23..16  red
//   bits 15..8   green
//   bits  7..0   blue
struct Color
{
    std::uint32_t argb = 0xFF000000; // Opaque black.

    constexpr Color() = default;

    constexpr explicit Color(std::uint32_t value)
        : argb{ value }
    {}

    constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
        : argb{ static_cast<std::uint32_t>(a) << 24 |
                static_cast<std::uint32_t>(r) << 16 |
                static_cast<std::uint32_t>(g) << 8 |
                static_cast<std::uint32_t>(b) }
    {}

    constexpr std::uint8_t A() const { return static_cast<std::uint8_t>(argb >> 24); }
    constexpr std::uint8_t R() const { return static_cast<std::uint8_t>(argb >> 16); }
    constexpr std::uint8_t G() const { return static_cast<std::uint8_t>(argb >> 8); }
    constexpr std::uint8_t B() const { return static_cast<std::uint8_t>(argb); }

    constexpr bool operator==(const Color&) const = default;
};

// A Color must be exactly one 32-bit pixel, so a row of Colors can be
// copied straight into an ARGB8888 texture.
static_assert(sizeof(Color) == 4);

static_assert(Color{ 0x12, 0x34, 0x56, 0x78 }.argb == 0x78123456);
static_assert(Color{ 0x78123456 }.R() == 0x12);

} // namespace rast
