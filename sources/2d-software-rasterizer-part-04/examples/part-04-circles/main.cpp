// Part 4: Circles
// Discs and rings of every small radius, concentric rings that grow, and a disc that follows the mouse.
// F1 shows the bounding box of every circle: green when it's completely inside the clip rectangle
// (drawn without checks), yellow when it's only partly inside (checked pixel by pixel).

#include <rast/Rasterizer.hpp>
#include <rast/Surface.hpp>
#include <rast/Window.hpp>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <numbers>
#include <print>

// The bounding box of a circle, and the color that shows how it is clipped.
void DrawBounds(rast::Rasterizer& rasterizer, rast::Point center, int radius)
{
    const rast::Rect bounds = rast::Rect::FromSize(center.x - radius, center.y - radius, 2 * radius, 2 * radius);
    const rast::Rect visible = rast::Intersect(bounds, rasterizer.ClipRect());
    if (visible.IsEmpty())
    {
        return; // Not drawn at all.
    }
    rasterizer.DrawRect(bounds, visible == bounds ? rast::Color::Lime : rast::Color::Yellow);
}

int main()
{
    constexpr int width = 320;
    constexpr int height = 180;

    auto window = rast::Window::Create("Part 4: Circles", width * 4, height * 4);
    if (!window)
    {
        std::println(stderr, "{}", window.error());
        return 1;
    }

    rast::Surface surface{ width, height };
    rast::Rasterizer rasterizer{ surface };

    bool showBounds = false;
    const auto start = std::chrono::steady_clock::now();

    while (window->PollEvents())
    {
        if (window->WasKeyPressed(rast::Key::F1))
        {
            showBounds = !showBounds;
        }

        const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();

        rasterizer.Clear(rast::Color::MidnightBlue);

        // Discs (top) and rings (bottom) with a radius of 1 to 8 pixels.
        int x = 12;
        for (int radius = 1; radius <= 8; ++radius)
        {
            rasterizer.FillCircle({ x, 20 }, radius, rast::Color::Gold);
            rasterizer.DrawCircle({ x, 44 }, radius, rast::Color::Gold);
            if (showBounds)
            {
                DrawBounds(rasterizer, { x, 20 }, radius);
                DrawBounds(rasterizer, { x, 44 }, radius);
            }
            x += 2 * radius + 6;
        }

        // Concentric rings that grow, and start again in the middle.
        const rast::Point ringsCenter{ 240, 110 };
        for (int i = 0; i < 6; ++i)
        {
            const float grow = std::fmod(10.0f * time + 15.0f * static_cast<float>(i), 90.0f);
            const int radius = static_cast<int>(grow);
            rasterizer.DrawCircle(ringsCenter, radius, rast::Color::DeepSkyBlue);
            if (showBounds)
            {
                DrawBounds(rasterizer, ringsCenter, radius);
            }
        }

        // A disc with an outline that follows the mouse, with a radius that pulses.
        rast::Point mouse = window->MousePosition();
        if (!surface.Bounds().Contains(mouse))
        {
            mouse = { 90, 120 };
        }
        const float pulse = 0.5f + 0.5f * std::sin(2.0f * std::numbers::pi_v<float> * 0.25f * time);
        const int radius = 12 + static_cast<int>(std::lround(28.0f * pulse));
        rasterizer.FillCircle(mouse, radius, rast::Color::Tomato);
        rasterizer.DrawCircle(mouse, radius, rast::Color::White);
        if (showBounds)
        {
            DrawBounds(rasterizer, mouse, radius);
        }

        window->Present(surface);
    }

    return 0;
}
