// Part 8: Blending
// Transparent shapes over a checkerboard: black bars from transparent to opaque, three circles that
// overlap, translucent rectangles, a triangle that fades out, and a glow that follows the mouse.
// F1 switches the blend state of the shapes: AlphaBlend, PremultipliedAlpha, Additive, Multiply, Screen or Opaque.
// F2 switches the format of the color target: B8G8R8A8Unorm or B8G8R8A8UnormSrgb (linear light).
// F3 splits the screen: B8G8R8A8Unorm on the left, B8G8R8A8UnormSrgb on the right.
// F4 measures how fast both formats blend, and prints the result.

#include <rast/Blend.hpp>
#include <rast/Rasterizer.hpp>
#include <rast/Surface.hpp>
#include <rast/Window.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <print>

// A blend state with a name, to print it.
struct NamedBlendState
{
    const char* name;
    rast::BlendState state;
};

// The blend states that F1 switches between.
constexpr std::array blendStates = {
    NamedBlendState{ "AlphaBlend", rast::BlendState::AlphaBlend() },
    NamedBlendState{ "PremultipliedAlpha", rast::BlendState::PremultipliedAlpha() },
    NamedBlendState{ "Additive", rast::BlendState::Additive() },
    NamedBlendState{ "Multiply", rast::BlendState::Multiply() },
    NamedBlendState{ "Screen", rast::BlendState::Screen() },
    NamedBlendState{ "Opaque", rast::BlendState::Opaque() },
};

const char* ToString(rast::PixelFormat format)
{
    return format == rast::PixelFormat::B8G8R8A8Unorm ? "B8G8R8A8Unorm" : "B8G8R8A8UnormSrgb";
}

// The same color with another alpha.
rast::Color WithAlpha(rast::Color color, std::uint8_t alpha)
{
    return rast::Color{ color.R(), color.G(), color.B(), alpha };
}

// A white glow: opaque in the middle, and fading to transparent at the edge.
rast::Surface MakeGlow(int size)
{
    rast::Surface glow{ size, size };
    auto pixels = glow.Pixels();
    const float radius = 0.5f * static_cast<float>(size);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            // The distance from the middle of the texture to the center of the texel, from 0 to 1.
            const float dx = (static_cast<float>(x) + 0.5f - radius) / radius;
            const float dy = (static_cast<float>(y) + 0.5f - radius) / radius;
            const float falloff = std::max(0.0f, 1.0f - std::sqrt(dx * dx + dy * dy));
            pixels[x, y] = WithAlpha(rast::Color::White, static_cast<std::uint8_t>(255.0f * falloff * falloff + 0.5f));
        }
    }
    return glow;
}

// A checkerboard of light and dark squares, so you can see through the transparent shapes.
void DrawCheckerboard(rast::Rasterizer& rasterizer)
{
    const rast::Rect bounds = rasterizer.ColorTarget().Bounds();
    for (int y = 0; y < bounds.bottom; y += 10)
    {
        for (int x = 0; x < bounds.right; x += 10)
        {
            const bool light = (x / 10 + y / 10) % 2 == 0;
            const rast::Color color = light ? rast::Color::Gainsboro : rast::Color::DarkGray;
            rasterizer.FillRect(rast::Rect::FromSize(x, y, 10, 10), color);
        }
    }
}

// Draws the shapes with the blend state. For PremultipliedAlpha, the colors are premultiplied first,
// in the format of the color target, so the result looks the same as with AlphaBlend.
void DrawShapes(rast::Rasterizer& rasterizer, const rast::BlendState& blendState, rast::Point mouse,
                const rast::Surface& glow, float time)
{
    const rast::PixelFormat format = rasterizer.ColorTarget().Format();
    const bool premultiplied = blendState == rast::BlendState::PremultipliedAlpha();
    auto color = [&](rast::Color c) { return premultiplied ? rast::Premultiply(c, format) : c; };

    rasterizer.SetBlendState(rast::BlendState::Opaque());
    DrawCheckerboard(rasterizer);
    rasterizer.SetBlendState(blendState);

    // White, and black bars over it with an alpha of 0, 1/4, 1/2, 3/4 and 1.
    rasterizer.FillRect({ 10, 10, 310, 30 }, color(rast::Color::White));
    for (int i = 0; i < 5; ++i)
    {
        const auto alpha = static_cast<std::uint8_t>(std::min(255, i * 64));
        rasterizer.FillRect(rast::Rect::FromSize(10 + i * 60, 10, 60, 20), color(WithAlpha(rast::Color::Black, alpha)));
    }

    // Three circles that overlap, each half transparent.
    rasterizer.FillCircle({ 50, 80 }, 28, color(WithAlpha(rast::Color::Red, 128)));
    rasterizer.FillCircle({ 80, 80 }, 28, color(WithAlpha(rast::Color::Lime, 128)));
    rasterizer.FillCircle({ 65, 106 }, 28, color(WithAlpha(rast::Color::Blue, 128)));

    // Translucent rectangles, and one that fades in and out.
    rasterizer.FillRect({ 120, 50, 180, 110 }, color(WithAlpha(rast::Color::Gold, 160)));
    rasterizer.FillRect({ 150, 80, 210, 140 }, color(WithAlpha(rast::Color::DeepSkyBlue, 160)));
    const auto pulse = static_cast<std::uint8_t>(127.5f + 127.5f * std::sin(2.0f * time));
    rasterizer.FillRect({ 135, 120, 175, 165 }, color(WithAlpha(rast::Color::HotPink, pulse)));

    // A triangle that fades out: opaque at two vertices, transparent at the third.
    rasterizer.FillTriangle(rast::Vertex{ { 225.0f, 50.0f }, color(rast::Color::DarkOrange) },
                            rast::Vertex{ { 310.0f, 100.0f }, color(WithAlpha(rast::Color::DarkOrange, 0)) },
                            rast::Vertex{ { 225.0f, 165.0f }, color(rast::Color::Purple) });

    // A glow at the mouse, or circling when the mouse is outside: always added to the colors below it.
    const rast::Rect bounds = rasterizer.ColorTarget().Bounds();
    const rast::Vec2 center = bounds.Contains(mouse)
                                  ? rast::Vec2{ static_cast<float>(mouse.x), static_cast<float>(mouse.y) }
                                  : rast::Vec2{ 240.0f + 40.0f * std::cos(time), 110.0f + 30.0f * std::sin(time) };
    const float half = 0.5f * static_cast<float>(glow.Width());
    const rast::Vertex topLeft{ { center.x - half, center.y - half }, rast::Color::Orange, { 0.0f, 0.0f } };
    const rast::Vertex topRight{ { center.x + half, center.y - half }, rast::Color::Orange, { 1.0f, 0.0f } };
    const rast::Vertex bottomRight{ { center.x + half, center.y + half }, rast::Color::Orange, { 1.0f, 1.0f } };
    const rast::Vertex bottomLeft{ { center.x - half, center.y + half }, rast::Color::Orange, { 0.0f, 1.0f } };
    rasterizer.SetBlendState(rast::BlendState::Additive());
    rasterizer.FillTriangle(topLeft, topRight, bottomRight, glow);
    rasterizer.FillTriangle(topLeft, bottomRight, bottomLeft, glow);
}

// Blends a translucent color over a whole surface, many times, and prints how long one million pixels take.
void MeasureBlending(rast::Surface& surface)
{
    rast::Rasterizer rasterizer{ surface };
    rasterizer.SetBlendState(rast::BlendState::AlphaBlend());
    constexpr int repeats = 50;
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < repeats; ++i)
    {
        rasterizer.FillRect(surface.Bounds(), rast::Color{ 200, 100, 50, 128 });
    }
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    const double pixels = static_cast<double>(repeats) * surface.Width() * surface.Height();
    std::println("{}: {:.2f} ms per million pixels", ToString(surface.Format()), elapsed.count() * 1e6 / pixels);
}

int main()
{
    constexpr int width = 320;
    constexpr int height = 180;

    auto window = rast::Window::Create("Part 8: Blending", width * 4, height * 4);
    if (!window)
    {
        std::println(stderr, "{}", window.error());
        return 1;
    }

    // Two color targets with the same size, one for each format.
    rast::Surface unorm{ width, height, rast::PixelFormat::B8G8R8A8Unorm };
    rast::Surface srgb{ width, height, rast::PixelFormat::B8G8R8A8UnormSrgb };
    rast::Rasterizer rasterizer{ unorm };

    const rast::Surface glow = MakeGlow(64);

    std::size_t blendIndex = 0; // The index of the blend state in blendStates.
    rast::Surface* target = &unorm;
    bool split = false;
    std::println("F1: blend state ({}), F2: format ({}), F3: split screen, F4: measure", blendStates[blendIndex].name,
                 ToString(target->Format()));

    const auto start = std::chrono::steady_clock::now();

    while (window->PollEvents())
    {
        if (window->WasKeyPressed(rast::Key::F1))
        {
            blendIndex = (blendIndex + 1) % blendStates.size();
            std::println("Blend state: {}", blendStates[blendIndex].name);
        }
        if (window->WasKeyPressed(rast::Key::F2))
        {
            target = target == &unorm ? &srgb : &unorm;
            std::println("Format: {}", ToString(target->Format()));
        }
        if (window->WasKeyPressed(rast::Key::F3))
        {
            split = !split;
        }
        if (window->WasKeyPressed(rast::Key::F4))
        {
            MeasureBlending(unorm);
            MeasureBlending(srgb);
        }

        const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
        const rast::Point mouse = window->MousePosition();
        const rast::BlendState& blendState = blendStates[blendIndex].state;

        if (!split)
        {
            rasterizer.SetColorTarget(*target);
            DrawShapes(rasterizer, blendState, mouse, glow, time);
            window->Present(*target);
            continue;
        }

        // Split screen: the same shapes in both formats, and the right half of the sRGB
        // surface copied over the right half of the Unorm surface, with a line between them.
        rasterizer.SetColorTarget(unorm);
        DrawShapes(rasterizer, blendState, mouse, glow, time);
        rasterizer.SetColorTarget(srgb);
        DrawShapes(rasterizer, blendState, mouse, glow, time);
        auto left = unorm.Pixels();
        auto right = srgb.Pixels();
        for (int y = 0; y < height; ++y)
        {
            for (int x = width / 2; x < width; ++x)
            {
                left[x, y] = right[x, y];
            }
        }
        rasterizer.SetColorTarget(unorm);
        rasterizer.SetBlendState(rast::BlendState::Opaque());
        rasterizer.FillRect({ width / 2, 0, width / 2 + 1, height }, rast::Color::Black);
        window->Present(unorm);
    }

    return 0;
}
