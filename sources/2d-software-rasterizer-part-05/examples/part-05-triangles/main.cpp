// Part 5: Triangles
// A fan of triangles that turns, and a strip of triangles that follows the mouse. The triangles of
// the fan go clockwise (front-facing), the triangles of the strip go counter-clockwise (back-facing).
// F1 shows the bounding box of every triangle. F2 shows how often every pixel is drawn:
// gray once, red more than once. F3 switches culling: none, back faces, front faces.

#include <rast/Rasterizer.hpp>
#include <rast/Surface.hpp>
#include <rast/Window.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numbers>
#include <print>
#include <vector>

struct Triangle
{
    rast::Vec2 v0;
    rast::Vec2 v1;
    rast::Vec2 v2;
    rast::Color color;
};

// A fan of triangles around a center: a wheel with spokes. Each triangle goes from the center
// to one point on the rim and then to the next one, which is clockwise on the screen.
void AddFan(std::vector<Triangle>& triangles, rast::Vec2 center, float radius, float angle)
{
    constexpr int count = 12;
    auto rim = [&](int i) {
        const float a = angle + 2.0f * std::numbers::pi_v<float> * static_cast<float>(i) / count;
        return center + rast::Vec2{ std::cos(a), std::sin(a) } * radius;
    };
    for (int i = 0; i < count; ++i)
    {
        triangles.push_back({ center, rim(i), rim(i + 1), i % 2 ? rast::Color::Gold : rast::Color::Tomato });
    }
}

// A strip of triangles along a wave. Each quad between the top and the bottom of the strip is
// split into two triangles that go counter-clockwise on the screen.
void AddStrip(std::vector<Triangle>& triangles, rast::Vec2 center, float time)
{
    constexpr int quads = 10;
    auto top = [&](int i) {
        const float x = static_cast<float>(i - quads / 2) * 14.0f;
        return center + rast::Vec2{ x, -12.0f + 8.0f * std::sin(0.4f * x + 2.0f * time) };
    };
    auto bottom = [&](int i) { return top(i) + rast::Vec2{ 0.0f, 24.0f }; };
    for (int i = 0; i < quads; ++i)
    {
        triangles.push_back({ top(i), bottom(i), top(i + 1), rast::Color::SteelBlue });
        triangles.push_back({ top(i + 1), bottom(i), bottom(i + 1), rast::Color::SkyBlue });
    }
}

// The bounding box of a triangle in whole pixels, like FillTriangle computes it.
rast::Rect Bounds(const Triangle& t)
{
    return { static_cast<int>(std::floor(std::min({ t.v0.x, t.v1.x, t.v2.x }))),
             static_cast<int>(std::floor(std::min({ t.v0.y, t.v1.y, t.v2.y }))),
             static_cast<int>(std::ceil(std::max({ t.v0.x, t.v1.x, t.v2.x }))),
             static_cast<int>(std::ceil(std::max({ t.v0.y, t.v1.y, t.v2.y }))) };
}

int main()
{
    constexpr int width = 320;
    constexpr int height = 180;

    auto window = rast::Window::Create("Part 5: Triangles", width * 4, height * 4);
    if (!window)
    {
        std::println(stderr, "{}", window.error());
        return 1;
    }

    rast::Surface surface{ width, height };
    rast::Rasterizer rasterizer{ surface };

    // For the overdraw view: every triangle is drawn into this surface on its own, and counted.
    rast::Surface single{ width, height };
    std::vector<std::uint8_t> drawCount(static_cast<std::size_t>(width) * height);

    bool showBounds = false;
    bool showOverdraw = false;
    std::vector<Triangle> triangles;
    const auto start = std::chrono::steady_clock::now();

    while (window->PollEvents())
    {
        if (window->WasKeyPressed(rast::Key::F1))
        {
            showBounds = !showBounds;
        }
        if (window->WasKeyPressed(rast::Key::F2))
        {
            showOverdraw = !showOverdraw;
        }
        if (window->WasKeyPressed(rast::Key::F3))
        {
            // None, then Back, then Front, then None again.
            switch (rasterizer.GetCullMode())
            {
            case rast::CullMode::None: rasterizer.SetCullMode(rast::CullMode::Back); break;
            case rast::CullMode::Back: rasterizer.SetCullMode(rast::CullMode::Front); break;
            case rast::CullMode::Front: rasterizer.SetCullMode(rast::CullMode::None); break;
            }
        }

        const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();

        // The strip follows the mouse, or stays in place while the mouse is outside the surface.
        const rast::Point mouse = window->MousePosition();
        const rast::Vec2 stripCenter = surface.Bounds().Contains(mouse)
                                           ? rast::Vec2{ static_cast<float>(mouse.x), static_cast<float>(mouse.y) }
                                           : rast::Vec2{ 245.0f, 120.0f };

        triangles.clear();
        AddFan(triangles, { 95.0f, 90.0f }, 70.0f, 0.3f * time);
        AddStrip(triangles, stripCenter, time);

        rasterizer.Clear(rast::Color::MidnightBlue);

        if (!showOverdraw)
        {
            for (const Triangle& t : triangles)
            {
                rasterizer.FillTriangle(t.v0, t.v1, t.v2, t.color);
            }
        }
        else
        {
            // Count how often every pixel is drawn, one triangle at a time.
            std::ranges::fill(drawCount, std::uint8_t{ 0 });
            rasterizer.SetColorTarget(single);
            for (const Triangle& t : triangles)
            {
                rasterizer.Clear(rast::Color::Transparent);
                rasterizer.FillTriangle(t.v0, t.v1, t.v2, rast::Color::White);
                const auto pixels = single.Data();
                for (std::size_t i = 0; i < pixels.size(); ++i)
                {
                    drawCount[i] += pixels[i] == rast::Color::White;
                }
            }
            rasterizer.SetColorTarget(surface);

            const auto pixels = surface.Data();
            for (std::size_t i = 0; i < pixels.size(); ++i)
            {
                if (drawCount[i] == 1)
                {
                    pixels[i] = rast::Color::Gray;
                }
                else if (drawCount[i] > 1)
                {
                    pixels[i] = rast::Color::Red;
                }
            }
        }

        if (showBounds)
        {
            for (const Triangle& t : triangles)
            {
                rasterizer.DrawRect(rast::Intersect(Bounds(t), rasterizer.ClipRect()), rast::Color::Yellow);
            }
        }

        window->Present(surface);
    }

    return 0;
}
