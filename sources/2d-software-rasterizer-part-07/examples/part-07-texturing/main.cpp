// Part 7: Texturing
// A little platformer scene, drawn with textured triangles: a background that repeats, a board of signs,
// tiles and a walking character from sprite sheets, a key that turns, and clouds tinted by
// their vertex colors. The images are from Kenney's Pixel Platformer (see assets/CREDITS.md).
// F1 switches the address mode of the board of signs: Wrap, Mirror or Clamp.
// F2 shows the texels: every texture becomes a checkerboard, one square per texel.

#include <rast/Rasterizer.hpp>
#include <rast/Sampler.hpp>
#include <rast/Surface.hpp>
#include <rast/Window.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <optional>
#include <print>
#include <string>
#include <utility>

// A rectangle of texture coordinates: from (u0, v0) at its top-left corner to (u1, v1) at its bottom-right corner.
struct TexRect
{
    float u0;
    float v0;
    float u1;
    float v1;
};

// The texture coordinates of a rectangle of texels, like one sprite in a sprite sheet.
TexRect ToTexCoords(const rast::Surface& texture, rast::Rect texels)
{
    const float width = static_cast<float>(texture.Width());
    const float height = static_cast<float>(texture.Height());
    return { static_cast<float>(texels.left) / width, static_cast<float>(texels.top) / height,
             static_cast<float>(texels.right) / width, static_cast<float>(texels.bottom) / height };
}

// The same texture coordinates, mirrored from left to right.
TexRect FlipHorizontally(TexRect r)
{
    return { r.u1, r.v0, r.u0, r.v1 };
}

// Draws a textured quad as two triangles. The corners go clockwise, starting at the top-left
// corner. The top corners get the color `top` and the bottom corners the color `bottom`.
void DrawQuad(rast::Rasterizer& rasterizer, const std::array<rast::Vec2, 4>& corners, TexRect r,
              const rast::Surface& texture, const rast::Sampler& sampler = {}, rast::Color top = rast::Color::White,
              rast::Color bottom = rast::Color::White)
{
    const rast::Vertex topLeft{ corners[0], top, { r.u0, r.v0 } };
    const rast::Vertex topRight{ corners[1], top, { r.u1, r.v0 } };
    const rast::Vertex bottomRight{ corners[2], bottom, { r.u1, r.v1 } };
    const rast::Vertex bottomLeft{ corners[3], bottom, { r.u0, r.v1 } };
    rasterizer.FillTriangle(topLeft, topRight, bottomRight, texture, sampler);
    rasterizer.FillTriangle(topLeft, bottomRight, bottomLeft, texture, sampler);
}

// The corners of a rectangle at a position: its top-left corner.
std::array<rast::Vec2, 4> Corners(rast::Vec2 position, float width, float height)
{
    return { position, position + rast::Vec2{ width, 0.0f }, position + rast::Vec2{ width, height },
             position + rast::Vec2{ 0.0f, height } };
}

// The corners of a rectangle around a center, turned by an angle (in radians, clockwise on the screen).
std::array<rast::Vec2, 4> TurnedCorners(rast::Vec2 center, float width, float height, float angle)
{
    // The directions of the rectangle's sides, half as long as the sides.
    const rast::Vec2 right = rast::Vec2{ std::cos(angle), std::sin(angle) } * (0.5f * width);
    const rast::Vec2 down = rast::Vec2{ -std::sin(angle), std::cos(angle) } * (0.5f * height);
    return { center - right - down, center + right - down, center + right + down, center - right + down };
}

// The 18 x 18 tile in a column and a row of tiles.png.
rast::Rect Tile(int column, int row)
{
    return rast::Rect::FromSize(column * 18, row * 18, 18, 18);
}

// The 24 x 24 character in a column and a row of characters.png.
rast::Rect Character(int column, int row)
{
    return rast::Rect::FromSize(column * 24, row * 24, 24, 24);
}

// A checkerboard as large as the texture, one square per texel, with the texture's alpha.
rast::Surface Checkerboard(const rast::Surface& texture)
{
    rast::Surface checkerboard{ texture.Width(), texture.Height() };
    auto source = texture.Pixels();
    auto pixels = checkerboard.Pixels();
    for (int y = 0; y < texture.Height(); ++y)
    {
        for (int x = 0; x < texture.Width(); ++x)
        {
            const rast::Color color = (x + y) % 2 == 0 ? rast::Color::White : rast::Color::SlateGray;
            pixels[x, y] = rast::Color{ color.R(), color.G(), color.B(), source[x, y].A() };
        }
    }
    return checkerboard;
}

// A copy of a rectangle of texels, as a texture of its own.
rast::Surface Crop(const rast::Surface& texture, rast::Rect texels)
{
    rast::Surface cropped{ texels.Width(), texels.Height() };
    auto source = texture.Pixels();
    auto pixels = cropped.Pixels();
    for (int y = 0; y < cropped.Height(); ++y)
    {
        for (int x = 0; x < cropped.Width(); ++x)
        {
            pixels[x, y] = source[texels.left + x, texels.top + y];
        }
    }
    return cropped;
}

// A texture, and the same texture as a checkerboard to show its texels.
struct Texture
{
    rast::Surface image;
    rast::Surface checkerboard;

    explicit Texture(rast::Surface texture)
        : image{ std::move(texture) }
        , checkerboard{ Checkerboard(image) }
    {}

    // The image, or the checkerboard when the texels are shown.
    const rast::Surface& Get(bool showTexels) const { return showTexels ? checkerboard : image; }
};

std::optional<Texture> LoadTexture(const std::string& path)
{
    auto image = rast::Surface::Load(path);
    if (!image)
    {
        std::println(stderr, "{}", image.error());
        return std::nullopt;
    }
    return Texture{ std::move(*image) };
}

const char* ToString(rast::AddressMode mode)
{
    switch (mode)
    {
    case rast::AddressMode::Wrap:
        return "Wrap";
    case rast::AddressMode::Mirror:
        return "Mirror";
    case rast::AddressMode::Clamp:
        return "Clamp";
    }
    return "";
}

int main()
{
    constexpr int width = 320;
    constexpr int height = 180;

    auto window = rast::Window::Create("Part 7: Texturing", width * 4, height * 4);
    if (!window)
    {
        std::println(stderr, "{}", window.error());
        return 1;
    }

    // The images are in the assets folder, next to the executable.
    const auto background = LoadTexture("assets/background.png");
    const auto tiles = LoadTexture("assets/tiles.png");
    const auto characters = LoadTexture("assets/characters.png");
    if (!background || !tiles || !characters)
    {
        return 1;
    }

    // The address modes work on a whole texture, so the sign gets a texture of its own.
    const Texture sign{ Crop(tiles->image, Tile(5, 4)) };

    rast::Surface surface{ width, height };
    rast::Rasterizer rasterizer{ surface };

    // The background repeats from left to right, but not from top to bottom.
    const rast::Sampler backgroundSampler{ rast::AddressMode::Wrap, rast::AddressMode::Clamp };
    rast::Sampler signSampler{ rast::AddressMode::Wrap, rast::AddressMode::Wrap };
    bool showTexels = false;
    std::println("F1: address mode of the signs ({})", ToString(signSampler.addressU));
    std::println("F2: show the texels");

    const auto start = std::chrono::steady_clock::now();

    while (window->PollEvents())
    {
        if (window->WasKeyPressed(rast::Key::F1))
        {
            switch (signSampler.addressU)
            {
            case rast::AddressMode::Wrap:
                signSampler = { rast::AddressMode::Mirror, rast::AddressMode::Mirror };
                break;
            case rast::AddressMode::Mirror:
                signSampler = { rast::AddressMode::Clamp, rast::AddressMode::Clamp };
                break;
            case rast::AddressMode::Clamp:
                signSampler = { rast::AddressMode::Wrap, rast::AddressMode::Wrap };
                break;
            }
            std::println("Address mode: {}", ToString(signSampler.addressU));
        }
        if (window->WasKeyPressed(rast::Key::F2))
        {
            showTexels = !showTexels;
        }

        // The textures to draw with: the images, or their checkerboards.
        const rast::Surface& backgroundTexture = background->Get(showTexels);
        const rast::Surface& tilesTexture = tiles->Get(showTexels);
        const rast::Surface& charactersTexture = characters->Get(showTexels);
        const rast::Surface& signTexture = sign.Get(showTexels);

        const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();

        // The background: one quad over the sky, 320 x 144 pixels. The image is a strip of
        // 48 x 144 texels, so u goes from 0 to 1 over the first 48 pixels, and from 0 to 320 / 48
        // over the whole quad: the sampler repeats the strip from left to right.
        DrawQuad(rasterizer, Corners({ 0.0f, 0.0f }, 320.0f, 144.0f), { 0.0f, 0.0f, 320.0f / 48.0f, 1.0f },
                 backgroundTexture, backgroundSampler);

        // The ground: two rows of tiles from the tile sheet, one quad per tile.
        for (int x = 0; x < width; x += 18)
        {
            const float left = static_cast<float>(x);
            DrawQuad(rasterizer, Corners({ left, 144.0f }, 18.0f, 18.0f), ToTexCoords(tilesTexture, Tile(2, 1)),
                     tilesTexture);
            DrawQuad(rasterizer, Corners({ left, 162.0f }, 18.0f, 18.0f), ToTexCoords(tilesTexture, Tile(2, 6)),
                     tilesTexture);
        }

        // A tree, a bush and a sign on the ground.
        DrawQuad(rasterizer, Corners({ 36.0f, 126.0f }, 18.0f, 18.0f), ToTexCoords(tilesTexture, Tile(6, 6)),
                 tilesTexture);
        DrawQuad(rasterizer, Corners({ 72.0f, 126.0f }, 18.0f, 18.0f), ToTexCoords(tilesTexture, Tile(4, 6)),
                 tilesTexture);
        DrawQuad(rasterizer, Corners({ 270.0f, 126.0f }, 18.0f, 18.0f), ToTexCoords(tilesTexture, Tile(5, 4)),
                 tilesTexture);

        // Clouds of three tiles, drawn twice as large, that drift to the right and come back
        // on the left. Their vertex colors tint them: white at the top, pink at the bottom.
        const float cloudX = std::round(std::fmod(8.0f * time, 320.0f + 108.0f)) - 108.0f;
        DrawQuad(rasterizer, Corners({ cloudX, 16.0f }, 108.0f, 36.0f),
                 ToTexCoords(tilesTexture, rast::Rect::FromSize(13 * 18, 7 * 18, 3 * 18, 18)), tilesTexture, {},
                 rast::Color::White, rast::Color::LightPink);

        // A board of signs, twice as large as the tile: the texture coordinates go from -1 to 2,
        // so the sign itself is in the middle, and the address mode decides what's around it.
        DrawQuad(rasterizer, Corners({ 16.0f, 24.0f }, 108.0f, 108.0f), { -1.0f, -1.0f, 2.0f, 2.0f }, signTexture,
                 signSampler);

        // A key that turns, three to five times as large as its tile.
        const float keyScale = 4.0f + std::sin(1.3f * time);
        DrawQuad(rasterizer, TurnedCorners({ 230.0f, 64.0f }, 18.0f * keyScale, 18.0f * keyScale, 0.7f * time),
                 ToTexCoords(tilesTexture, Tile(7, 1)), tilesTexture);

        // A character that walks to the right and back, with two frames of animation. The frames
        // look to the left, so walking to the right, it flips them with its texture coordinates.
        constexpr float walkLeft = 20.0f;
        constexpr float walkRight = 276.0f;
        const float distance = std::fmod(40.0f * time, 2.0f * (walkRight - walkLeft));
        const bool walkingRight = distance < walkRight - walkLeft;
        const float characterX =
            std::round(walkingRight ? walkLeft + distance : walkRight - (distance - (walkRight - walkLeft)));
        const int frame = static_cast<int>(6.0f * time) % 2;
        const TexRect character = ToTexCoords(charactersTexture, Character(frame, 0));
        DrawQuad(rasterizer, Corners({ characterX, 120.0f }, 24.0f, 24.0f),
                 walkingRight ? FlipHorizontally(character) : character, charactersTexture);

        window->Present(surface);
    }

    return 0;
}
