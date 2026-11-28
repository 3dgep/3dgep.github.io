// Part 2: Clipping
// Rectangles slide over the edges of the surface. The scene is drawn twice:
// dimmed on the whole surface, and in full color inside a scissor rectangle
// around the mouse. F1 shows the outline of the scissor rectangle.

#include <rast/Rasterizer.hpp>
#include <rast/Surface.hpp>
#include <rast/Window.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <print>

// A rectangle that moves back and forth, and partly off the surface.
struct MovingRect
{
    int width;
    int height;
    float centerX; // The middle of its path, in pixels.
    float centerY;
    float rangeX;  // How far it moves away from the middle, in pixels.
    float rangeY;
    float speedX;  // How fast it moves, in radians per second.
    float speedY;
    rast::Color color;
};

constexpr std::array movingRects = {
    MovingRect{ 200, 110, 320.0f, 90.0f, 330.0f, 40.0f, 0.7f, 1.3f, rast::Color{ 0xE0, 0x50, 0x50 } },
    MovingRect{ 140, 140, 320.0f, 180.0f, 120.0f, 230.0f, 1.1f, 0.8f, rast::Color{ 0x50, 0xC0, 0x60 } },
    MovingRect{ 260, 80, 320.0f, 280.0f, 360.0f, 60.0f, 0.5f, 1.7f, rast::Color{ 0x50, 0x80, 0xE0 } },
    MovingRect{ 120, 200, 480.0f, 180.0f, 250.0f, 200.0f, 0.9f, 0.6f, rast::Color{ 0xF0, 0xA0, 0x30 } },
    MovingRect{ 90, 90, 160.0f, 200.0f, 220.0f, 180.0f, 1.5f, 1.0f, rast::Color{ 0xB0, 0x60, 0xD0 } },
};

// The rectangle's position at a moment in time.
rast::Rect RectAt(const MovingRect& moving, float time)
{
    const float x = moving.centerX + moving.rangeX * std::sin(moving.speedX * time);
    const float y = moving.centerY + moving.rangeY * std::cos(moving.speedY * time);

    // Round to the nearest pixel. A static_cast would round towards zero,
    // which moves negative coordinates the wrong way.
    const int left = static_cast<int>(std::lround(x)) - moving.width / 2;
    const int top = static_cast<int>(std::lround(y)) - moving.height / 2;
    return rast::Rect::FromSize(left, top, moving.width, moving.height);
}

// A darker version of a color.
rast::Color Dim(rast::Color color)
{
    return rast::Color{ static_cast<std::uint8_t>(color.R() / 3), static_cast<std::uint8_t>(color.G() / 3),
                        static_cast<std::uint8_t>(color.B() / 3) };
}

void DrawScene(rast::Rasterizer& rasterizer, float time, bool dimmed)
{
    const rast::Color outline{ 0xFF, 0xFF, 0xFF };

    for (const MovingRect& moving : movingRects)
    {
        const rast::Rect rect = RectAt(moving, time);
        rasterizer.FillRect(rect, dimmed ? Dim(moving.color) : moving.color);
        rasterizer.DrawRect(rect, dimmed ? Dim(outline) : outline);
    }
}

int main()
{
    constexpr int width = 640;
    constexpr int height = 360;

    auto window = rast::Window::Create("Part 2: Clipping", width * 2, height * 2);
    if (!window)
    {
        std::println(stderr, "{}", window.error());
        return 1;
    }

    rast::Surface surface{ width, height };
    rast::Rasterizer rasterizer{ surface };

    bool showScissor = false;
    const auto start = std::chrono::steady_clock::now();

    while (window->PollEvents())
    {
        if (window->WasKeyPressed(rast::Key::F1))
        {
            showScissor = !showScissor;
        }

        const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();

        // First the whole surface, dimmed.
        rasterizer.ResetScissor();
        rasterizer.Clear(rast::Color{ 0x18, 0x18, 0x20 });
        DrawScene(rasterizer, time, true);

        // Then the spotlight: only the pixels inside the scissor rectangle change.
        const rast::Point mouse = window->MousePosition();
        const rast::Rect spotlight = rast::Rect::FromSize(mouse.x - 100, mouse.y - 75, 200, 150);
        rasterizer.SetScissor(spotlight);
        rasterizer.Clear(rast::Color{ 0x38, 0x38, 0x48 });
        DrawScene(rasterizer, time, false);

        // A debug overlay: the outline of the scissor rectangle.
        if (showScissor)
        {
            rasterizer.ResetScissor();
            rasterizer.DrawRect(spotlight, rast::Color{ 0xFF, 0xE0, 0x00 });
        }

        window->Present(surface);
    }

    return 0;
}
