#pragma once

#include "Color.hpp"
#include "Surface.hpp"

#include <cstdint>

namespace rast
{

// What a color is multiplied by before the blend operation combines it with the other one
// (D3D12_BLEND in Direct3D 12, VkBlendFactor in Vulkan). Src is the source, the color that is
// drawn, and Dest is the destination, the color that is already in the color target. For the
// alpha channel, the Color factors use the alpha of that color.
enum class BlendFactor
{
    Zero,         // 0
    One,          // 1
    SrcColor,     // The source's red, green or blue.
    InvSrcColor,  // 1 - the source's red, green or blue.
    SrcAlpha,     // The source's alpha.
    InvSrcAlpha,  // 1 - the source's alpha.
    DestColor,    // The destination's red, green or blue.
    InvDestColor, // 1 - the destination's red, green or blue.
    DestAlpha,    // The destination's alpha.
    InvDestAlpha, // 1 - the destination's alpha.
};

// How the weighted source and destination are combined (D3D12_BLEND_OP, VkBlendOp).
// Min and Max ignore the factors, like in Direct3D and Vulkan.
enum class BlendOp
{
    Add,             // source + destination
    Subtract,        // source - destination
    ReverseSubtract, // destination - source
    Min,             // The smaller of the two.
    Max,             // The larger of the two.
};

// How a color that is drawn is blended with the color target, like D3D12_RENDER_TARGET_BLEND_DESC:
//
//   result = (source * srcBlend) blendOp (destination * destBlend)
//
// for red, green and blue, and the same with the Alpha members for alpha. The result is clamped
// to the range from 0 to 1.
struct BlendState
{
    bool blendEnable = false; // Without blending, the source replaces the destination.
    BlendFactor srcBlend = BlendFactor::One;
    BlendFactor destBlend = BlendFactor::Zero;
    BlendOp blendOp = BlendOp::Add;
    BlendFactor srcBlendAlpha = BlendFactor::One;
    BlendFactor destBlendAlpha = BlendFactor::Zero;
    BlendOp blendOpAlpha = BlendOp::Add;

    constexpr bool operator==(const BlendState&) const = default;

    // No blending: the source replaces the destination.
    static constexpr BlendState Opaque() { return {}; }

    // Source over destination, for straight colors: source * a + destination * (1 - a).
    static constexpr BlendState AlphaBlend()
    {
        return { .blendEnable = true,
                 .srcBlend = BlendFactor::SrcAlpha,
                 .destBlend = BlendFactor::InvSrcAlpha,
                 .destBlendAlpha = BlendFactor::InvSrcAlpha };
    }

    // Source over destination, for premultiplied colors: source + destination * (1 - a).
    static constexpr BlendState PremultipliedAlpha()
    {
        return { .blendEnable = true,
                 .srcBlend = BlendFactor::One,
                 .destBlend = BlendFactor::InvSrcAlpha,
                 .destBlendAlpha = BlendFactor::InvSrcAlpha };
    }

    // Adds light: destination + source * a.
    static constexpr BlendState Additive()
    {
        return { .blendEnable = true,
                 .srcBlend = BlendFactor::SrcAlpha,
                 .destBlend = BlendFactor::One,
                 .destBlendAlpha = BlendFactor::InvSrcAlpha };
    }

    // Darkens: source * destination, like a colored filter.
    static constexpr BlendState Multiply()
    {
        return { .blendEnable = true,
                 .srcBlend = BlendFactor::DestColor,
                 .destBlend = BlendFactor::Zero,
                 .destBlendAlpha = BlendFactor::InvSrcAlpha };
    }

    // Lightens: source + destination * (1 - source), the opposite of Multiply.
    static constexpr BlendState Screen()
    {
        return { .blendEnable = true,
                 .srcBlend = BlendFactor::One,
                 .destBlend = BlendFactor::InvSrcColor,
                 .destBlendAlpha = BlendFactor::InvSrcAlpha };
    }
};

// The source color blended with the destination color, in a color target with the given format.
// B8G8R8A8Unorm blends the bytes as they are, with integer math. B8G8R8A8UnormSrgb converts the
// red, green and blue bytes to linear light, blends them with floats, and converts the result back.
// Alpha isn't light, so it's always blended as a byte.
Color Blend(Color source, Color destination, const BlendState& state, PixelFormat format);

// The color with its red, green and blue multiplied by its alpha, for BlendState::PremultipliedAlpha.
// format is the format of the color target that the color is blended into: in B8G8R8A8UnormSrgb,
// the multiplication happens in linear light, like the blending.
Color Premultiply(Color color, PixelFormat format);

// Premultiplies every pixel of a surface (like a texture), for a color target with the given format.
void Premultiply(Surface& surface, PixelFormat format);

// Converts one channel between an sRGB byte and linear light, from 0 to 1.
float SrgbToLinear(std::uint8_t value);
std::uint8_t LinearToSrgb(float value);

} // namespace rast
