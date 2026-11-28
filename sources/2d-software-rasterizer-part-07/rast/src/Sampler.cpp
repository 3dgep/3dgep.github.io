#include <rast/Sampler.hpp>

#include <algorithm>
#include <cmath>

namespace rast
{

namespace
{

// The texel that the coordinate falls in, along one axis of a texture that is `size` texels long.
// Texel i covers the coordinates from i / size up to (i + 1) / size.
int TexelIndex(float coordinate, int size)
{
    const float index = std::floor(coordinate * static_cast<float>(size));

    // Keep the index in the range of an int, so the conversion below is always defined.
    // A coordinate this far outside the texture has no precision left anyway.
    constexpr float limit = 1 << 30;
    return static_cast<int>(std::clamp(index, -limit, limit));
}

// a modulo b, but never negative (unlike a % b): the result is from 0 up to b. b must be positive.
int Modulo(int a, int b)
{
    const int remainder = a % b;
    return remainder < 0 ? remainder + b : remainder;
}

// Turns a texel index, which may be outside the texture, into one inside it: 0 up to size.
int ApplyAddressMode(int index, int size, AddressMode mode)
{
    switch (mode)
    {
    case AddressMode::Wrap:
        return Modulo(index, size);
    case AddressMode::Mirror:
    {
        // The texture and its mirror image repeat together, every 2 * size texels.
        const int i = Modulo(index, 2 * size);
        return i < size ? i : 2 * size - 1 - i;
    }
    case AddressMode::Clamp:
        return std::clamp(index, 0, size - 1);
    }
    return 0;
}

} // namespace

Color Sample(const Surface& texture, const Sampler& sampler, Vec2 texCoord)
{
    const int x = ApplyAddressMode(TexelIndex(texCoord.x, texture.Width()), texture.Width(), sampler.addressU);
    const int y = ApplyAddressMode(TexelIndex(texCoord.y, texture.Height()), texture.Height(), sampler.addressV);
    return texture.Pixels()[x, y];
}

} // namespace rast
