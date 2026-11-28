// Part 3: Lines
// A grid and a starburst of lines around the mouse, drawn with Bresenham's algorithm.
// F1 switches to the DDA. F2 shows where the two algorithms draw different pixels.

#include <rast/Rasterizer.hpp>
#include <rast/Surface.hpp>
#include <rast/Window.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <numbers>
#include <print>

// The DDA (digital differential analyzer): take one step per pixel along the axis with the
// larger distance, and move along the other axis by the slope. Half-open, like DrawLine.
// It checks every pixel with SetPixel, which is fine for a comparison, but not for a library.
void DrawLineDDA(rast::Rasterizer& rasterizer, rast::Point p0, rast::Point p1, rast::Color color)
{
    const int dx = p1.x - p0.x;
    const int dy = p1.y - p0.y;
    const int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0)
    {
        return;
    }

    const float stepX = static_cast<float>(dx) / static_cast<float>(steps);
    const float stepY = static_cast<float>(dy) / static_cast<float>(steps);

    float x = static_cast<float>(p0.x);
    float y = static_cast<float>(p0.y);
    for (int i = 0; i < steps; ++i)
    {
        rasterizer.SetPixel({ static_cast<int>(std::lround(x)), static_cast<int>(std::lround(y)) }, color);
        x += stepX;
        y += stepY;
    }
}

enum class Algorithm
{
    Bresenham,
    DDA,
};

// Lines from the center outwards, in every direction, slowly turning.
void DrawStarburst(rast::Rasterizer& rasterizer, rast::Point center, float time, Algorithm algorithm,
                   rast::Color color)
{
    constexpr int lineCount = 48;
    constexpr float length = 120.0f;

    for (int i = 0; i < lineCount; ++i)
    {
        const float angle = 0.1f * time + 2.0f * std::numbers::pi_v<float> * static_cast<float>(i) / lineCount;
        const rast::Point end{ center.x + static_cast<int>(std::lround(length * std::cos(angle))),
                               center.y + static_cast<int>(std::lround(length * std::sin(angle))) };

        if (algorithm == Algorithm::Bresenham)
        {
            rasterizer.DrawLine(center, end, color);
        }
        else
        {
            DrawLineDDA(rasterizer, center, end, color);
        }
    }
}

// A grid of horizontal and vertical lines.
void DrawGrid(rast::Rasterizer& rasterizer, int spacing, rast::Color color)
{
    const rast::Rect bounds = rasterizer.ColorTarget().Bounds();
    for (int x = 0; x < bounds.right; x += spacing)
    {
        rasterizer.DrawLine({ x, 0 }, { x, bounds.bottom }, color);
    }
    for (int y = 0; y < bounds.bottom; y += spacing)
    {
        rasterizer.DrawLine({ 0, y }, { bounds.right, y }, color);
    }
}

int main()
{
    constexpr int width = 320;
    constexpr int height = 180;

    auto window = rast::Window::Create("Part 3: Lines", width * 4, height * 4);
    if (!window)
    {
        std::println(stderr, "{}", window.error());
        return 1;
    }

    rast::Surface surface{ width, height };
    rast::Rasterizer rasterizer{ surface };

    // For the comparison: each algorithm draws the starburst into a surface of its own.
    rast::Surface bresenhamLines{ width, height };
    rast::Surface ddaLines{ width, height };

    Algorithm algorithm = Algorithm::Bresenham;
    bool showDifferences = false;
    const auto start = std::chrono::steady_clock::now();

    while (window->PollEvents())
    {
        if (window->WasKeyPressed(rast::Key::F1))
        {
            algorithm = algorithm == Algorithm::Bresenham ? Algorithm::DDA : Algorithm::Bresenham;
        }
        if (window->WasKeyPressed(rast::Key::F2))
        {
            showDifferences = !showDifferences;
        }

        const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();

        // The starburst follows the mouse, or stays in the middle while the mouse is outside the surface.
        rast::Point center = window->MousePosition();
        if (!surface.Bounds().Contains(center))
        {
            center = { width / 2, height / 2 };
        }

        rasterizer.Clear(rast::Color::Black);
        DrawGrid(rasterizer, 20, rast::Color::DarkSlateGray);

        if (!showDifferences)
        {
            DrawStarburst(rasterizer, center, time, algorithm, rast::Color::White);
        }
        else
        {
            // Draw the starburst with both algorithms, each into its own surface.
            rasterizer.SetColorTarget(bresenhamLines);
            rasterizer.Clear(rast::Color::Transparent);
            DrawStarburst(rasterizer, center, time, Algorithm::Bresenham, rast::Color::White);

            rasterizer.SetColorTarget(ddaLines);
            rasterizer.Clear(rast::Color::Transparent);
            DrawStarburst(rasterizer, center, time, Algorithm::DDA, rast::Color::White);

            rasterizer.SetColorTarget(surface);

            // Compare them pixel by pixel: gray where both agree, colored where only one drew a pixel.
            const auto bresenham = bresenhamLines.Data();
            const auto dda = ddaLines.Data();
            const auto pixels = surface.Data();
            for (std::size_t i = 0; i < pixels.size(); ++i)
            {
                const bool inBresenham = bresenham[i] != rast::Color::Transparent;
                const bool inDDA = dda[i] != rast::Color::Transparent;
                if (inBresenham && inDDA)
                {
                    pixels[i] = rast::Color::Gray;
                }
                else if (inBresenham)
                {
                    pixels[i] = rast::Color::Lime;
                }
                else if (inDDA)
                {
                    pixels[i] = rast::Color::Red;
                }
            }
        }

        window->Present(surface);
    }

    return 0;
}
