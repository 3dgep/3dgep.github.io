#include <rast/Blend.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace rast
{

namespace
{

// x / 255, rounded to the nearest whole number, without a division. Exact for every x
// from 0 to 255 * 255, the largest product of two bytes (Jim Blinn's "three wrongs make a right").
constexpr std::uint32_t DivideBy255(std::uint32_t x)
{
    x += 128;
    return (x + (x >> 8)) >> 8;
}

static_assert(DivideBy255(0) == 0);
static_assert(DivideBy255(255 * 255) == 255);
// 127 / 255 is just below one half, and 128 / 255 just above it.
static_assert(DivideBy255(127) == 0 && DivideBy255(128) == 1);

// The sRGB transfer functions, for one channel from 0 to 1.
float DecodeSrgb(float c)
{
    return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}

float EncodeSrgb(float l)
{
    return l <= 0.0031308f ? 12.92f * l : 1.055f * std::pow(l, 1.0f / 2.4f) - 0.055f;
}

// Decoding: every byte has its own entry, so the table is exact.
const std::array<float, 256> DecodeTable = [] {
    std::array<float, 256> table{};
    for (std::size_t i = 0; i < table.size(); ++i)
    {
        table[i] = DecodeSrgb(static_cast<float>(i) / 255.0f);
    }
    return table;
}();

// Encoding: linear light is split into 4096 steps, enough to give every sRGB byte at least one step,
// even the darkest ones, where sRGB is most precise.
constexpr std::size_t EncodeSteps = 4096;
const std::array<std::uint8_t, EncodeSteps> EncodeTable = [] {
    std::array<std::uint8_t, EncodeSteps> table{};
    for (std::size_t i = 0; i < table.size(); ++i)
    {
        const float l = static_cast<float>(i) / static_cast<float>(EncodeSteps - 1);
        table[i] = static_cast<std::uint8_t>(EncodeSrgb(l) * 255.0f + 0.5f);
    }
    return table;
}();

// One channel of the source and of the destination, and the alphas of both colors: from 0 to 255
// for bytes, and from 0 to 1 in linear light.
template <typename T>
struct Channel
{
    T source;
    T destination;
    T sourceAlpha;
    T destinationAlpha;
};

// The value of a blend factor for one channel. one is 255 for bytes, and 1 in linear light.
template <typename T>
T Factor(BlendFactor factor, const Channel<T>& c, T one)
{
    switch (factor)
    {
    case BlendFactor::Zero:
        return T{ 0 };
    case BlendFactor::One:
        return one;
    case BlendFactor::SrcColor:
        return c.source;
    case BlendFactor::InvSrcColor:
        return one - c.source;
    case BlendFactor::SrcAlpha:
        return c.sourceAlpha;
    case BlendFactor::InvSrcAlpha:
        return one - c.sourceAlpha;
    case BlendFactor::DestColor:
        return c.destination;
    case BlendFactor::InvDestColor:
        return one - c.destination;
    case BlendFactor::DestAlpha:
        return c.destinationAlpha;
    case BlendFactor::InvDestAlpha:
        return one - c.destinationAlpha;
    }
    return T{ 0 };
}

// Blends one channel as bytes, with integers: the red, green and blue of the B8G8R8A8Unorm format,
// and alpha in both formats.
std::uint8_t BlendBytes(const Channel<std::uint32_t>& c, BlendFactor srcBlend, BlendFactor destBlend, BlendOp op)
{
    // The weighted values are 255 times too large, until they are divided by 255 at the end.
    const std::uint32_t s = c.source * Factor(srcBlend, c, 255u);
    const std::uint32_t d = c.destination * Factor(destBlend, c, 255u);
    switch (op)
    {
    case BlendOp::Add:
        // A sum of 255 * 255 or more is 255 or more after the division, so it's clamped first.
        return static_cast<std::uint8_t>(DivideBy255(std::min(s + d, 255u * 255u)));
    case BlendOp::Subtract:
        return static_cast<std::uint8_t>(s > d ? DivideBy255(s - d) : 0);
    case BlendOp::ReverseSubtract:
        return static_cast<std::uint8_t>(d > s ? DivideBy255(d - s) : 0);
    case BlendOp::Min:
        return static_cast<std::uint8_t>(std::min(c.source, c.destination));
    case BlendOp::Max:
        return static_cast<std::uint8_t>(std::max(c.source, c.destination));
    }
    return 0;
}

// Blends one channel in linear light, with floats: the red, green and blue of the
// B8G8R8A8UnormSrgb format.
std::uint8_t BlendLinear(const Channel<float>& c, BlendFactor srcBlend, BlendFactor destBlend, BlendOp op)
{
    const float s = c.source * Factor(srcBlend, c, 1.0f);
    const float d = c.destination * Factor(destBlend, c, 1.0f);
    float result = 0.0f;
    switch (op)
    {
    case BlendOp::Add:
        result = s + d;
        break;
    case BlendOp::Subtract:
        result = s - d;
        break;
    case BlendOp::ReverseSubtract:
        result = d - s;
        break;
    case BlendOp::Min:
        result = std::min(c.source, c.destination);
        break;
    case BlendOp::Max:
        result = std::max(c.source, c.destination);
        break;
    }
    return LinearToSrgb(result); // Clamps the result to the range from 0 to 1.
}

} // namespace

float SrgbToLinear(std::uint8_t value)
{
    return DecodeTable[value];
}

std::uint8_t LinearToSrgb(float value)
{
    // Clamp first: blending can go past 1 (with Add) or below 0 (with Subtract).
    const float steps = std::clamp(value, 0.0f, 1.0f) * static_cast<float>(EncodeSteps - 1);
    return EncodeTable[static_cast<std::size_t>(steps + 0.5f)];
}

Color Blend(Color source, Color destination, const BlendState& state, PixelFormat format)
{
    if (!state.blendEnable)
    {
        return source;
    }

    // Alpha is always blended as a byte.
    const std::uint32_t sourceAlpha = source.A();
    const std::uint32_t destinationAlpha = destination.A();
    auto bytes = [&](std::uint8_t s, std::uint8_t d, BlendFactor srcBlend, BlendFactor destBlend, BlendOp op) {
        return BlendBytes({ s, d, sourceAlpha, destinationAlpha }, srcBlend, destBlend, op);
    };
    const std::uint8_t alpha =
        bytes(source.A(), destination.A(), state.srcBlendAlpha, state.destBlendAlpha, state.blendOpAlpha);

    if (format == PixelFormat::B8G8R8A8Unorm)
    {
        return Color{ bytes(source.R(), destination.R(), state.srcBlend, state.destBlend, state.blendOp),
                      bytes(source.G(), destination.G(), state.srcBlend, state.destBlend, state.blendOp),
                      bytes(source.B(), destination.B(), state.srcBlend, state.destBlend, state.blendOp), alpha };
    }

    // In linear light, the factors and the operation work on the decoded colors.
    const float sourceAlphaF = static_cast<float>(sourceAlpha) / 255.0f;
    const float destinationAlphaF = static_cast<float>(destinationAlpha) / 255.0f;
    auto linear = [&](std::uint8_t s, std::uint8_t d) {
        return BlendLinear({ DecodeTable[s], DecodeTable[d], sourceAlphaF, destinationAlphaF }, state.srcBlend,
                           state.destBlend, state.blendOp);
    };
    return Color{ linear(source.R(), destination.R()), linear(source.G(), destination.G()),
                  linear(source.B(), destination.B()), alpha };
}

Color Premultiply(Color color, PixelFormat format)
{
    const std::uint32_t a = color.A();
    if (format == PixelFormat::B8G8R8A8Unorm)
    {
        auto channel = [a](std::uint32_t c) { return static_cast<std::uint8_t>(DivideBy255(c * a)); };
        return Color{ channel(color.R()), channel(color.G()), channel(color.B()), color.A() };
    }

    const float alpha = static_cast<float>(a) / 255.0f;
    auto channel = [alpha](std::uint8_t c) { return LinearToSrgb(SrgbToLinear(c) * alpha); };
    return Color{ channel(color.R()), channel(color.G()), channel(color.B()), color.A() };
}

void Premultiply(Surface& surface, PixelFormat format)
{
    for (Color& pixel : surface.Data())
    {
        pixel = Premultiply(pixel, format);
    }
}

} // namespace rast
