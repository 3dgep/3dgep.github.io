#pragma once

#include "Color.hpp"
#include "Surface.hpp"
#include "Vec2.hpp"

namespace rast
{

// What a sampler does with a texture coordinate outside the texture, below 0 or above 1.
// The names are the ones that Direct3D uses (D3D12_TEXTURE_ADDRESS_MODE);
// Vulkan calls them REPEAT, MIRRORED_REPEAT and CLAMP_TO_EDGE.
enum class AddressMode
{
    Wrap,   // Repeat the texture: 1.25 is the same as 0.25.
    Mirror, // Repeat the texture, flipped every other time: 1.25 is the same as 0.75.
    Clamp,  // Stretch the texels at the edges: 1.25 is the same as 1.
};

// How to read a color from a texture at a texture coordinate.
// addressU is for the horizontal coordinate (u), addressV for the vertical one (v).
struct Sampler
{
    AddressMode addressU = AddressMode::Wrap;
    AddressMode addressV = AddressMode::Wrap;
};

// The color of the texture at the texture coordinate: (0, 0) is the top-left corner of the
// texture, and (1, 1) the bottom-right corner. Nearest-neighbor sampling: the color of the
// texel (texture pixel) that the coordinate falls in.
Color Sample(const Surface& texture, const Sampler& sampler, Vec2 texCoord);

} // namespace rast
