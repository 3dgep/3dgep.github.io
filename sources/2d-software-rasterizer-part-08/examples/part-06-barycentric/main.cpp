// Part 6: Barycentric Coordinates
// A triangle with a red, a green and a blue vertex that turns, and a hexagon with a white center
// and a rainbow rim that follows the mouse. The colors are blended across the triangles.
// F1 switches to flat shading: one color per triangle, the average of its vertex colors.
// F2 shows the barycentric coordinates: red, green and blue at v0, v1 and v2 of every triangle.

#include <rast/Rasterizer.hpp>
#include <rast/Surface.hpp>
#include <rast/Window.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numbers>
#include <print>
#include <vector>

struct Triangle
{
    rast::Vertex v0;
    rast::Vertex v1;
    rast::Vertex v2;
};

// A point on a circle around a center.
rast::Vec2 OnCircle(rast::Vec2 center, float radius, float angle)
{
    return center + rast::Vec2{ std::cos(angle), std::sin(angle) } * radius;
}

// The classic triangle with a red, a green and a blue vertex.
void AddRgbTriangle(std::vector<Triangle>& triangles, rast::Vec2 center, float angle)
{
    constexpr float third = 2.0f * std::numbers::pi_v<float> / 3.0f;
    triangles.push_back({ { OnCircle(center, 75.0f, angle), rast::Color::Red },
                          { OnCircle(center, 75.0f, angle + third), rast::Color::Lime },
                          { OnCircle(center, 75.0f, angle + 2.0f * third), rast::Color::Blue } });
}

// A hexagon of six triangles around a white center. Neighboring triangles share a rim vertex,
// with the same color, so the colors flow from one triangle into the next.
void AddHexagon(std::vector<Triangle>& triangles, rast::Vec2 center)
{
    constexpr std::array rim = { rast::Color::Red,  rast::Color::Yellow, rast::Color::Lime,
                                 rast::Color::Cyan, rast::Color::Blue,   rast::Color::Magenta };
    const rast::Vertex middle{ center, rast::Color::White };
    for (std::size_t i = 0; i < rim.size(); ++i)
    {
        const float angle = std::numbers::pi_v<float> / 3.0f * static_cast<float>(i);
        const float next = std::numbers::pi_v<float> / 3.0f * static_cast<float>(i + 1);
        triangles.push_back({ middle,
                              { OnCircle(center, 60.0f, angle), rim[i] },
                              { OnCircle(center, 60.0f, next), rim[(i + 1) % rim.size()] } });
    }
}

// The average of the three vertex colors, for flat shading.
rast::Color Average(const Triangle& t)
{
    auto average = [](int a, int b, int c) { return static_cast<std::uint8_t>((a + b + c + 1) / 3); };
    return rast::Color{ average(t.v0.color.R(), t.v1.color.R(), t.v2.color.R()),
                        average(t.v0.color.G(), t.v1.color.G(), t.v2.color.G()),
                        average(t.v0.color.B(), t.v1.color.B(), t.v2.color.B()) };
}

enum class Shading
{
    Gouraud,
    Flat,
    Weights,
};

int main()
{
    constexpr int width = 320;
    constexpr int height = 180;

    auto window = rast::Window::Create("Part 6: Barycentric Coordinates", width * 4, height * 4);
    if (!window)
    {
        std::println(stderr, "{}", window.error());
        return 1;
    }

    rast::Surface surface{ width, height };
    rast::Rasterizer rasterizer{ surface };

    Shading shading = Shading::Gouraud;
    std::vector<Triangle> triangles;
    const auto start = std::chrono::steady_clock::now();

    while (window->PollEvents())
    {
        // F1 and F2 switch to flat shading or the barycentric coordinates, and back.
        if (window->WasKeyPressed(rast::Key::F1))
        {
            shading = shading == Shading::Flat ? Shading::Gouraud : Shading::Flat;
        }
        if (window->WasKeyPressed(rast::Key::F2))
        {
            shading = shading == Shading::Weights ? Shading::Gouraud : Shading::Weights;
        }

        const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();

        // The hexagon follows the mouse, or stays in place while the mouse is outside the surface.
        const rast::Point mouse = window->MousePosition();
        const rast::Vec2 hexagonCenter = surface.Bounds().Contains(mouse)
                                             ? rast::Vec2{ static_cast<float>(mouse.x), static_cast<float>(mouse.y) }
                                             : rast::Vec2{ 235.0f, 90.0f };

        triangles.clear();
        AddRgbTriangle(triangles, { 85.0f, 92.0f }, 0.4f * time);
        AddHexagon(triangles, hexagonCenter);

        rasterizer.Clear(rast::Color::Black);

        for (const Triangle& t : triangles)
        {
            switch (shading)
            {
            case Shading::Gouraud:
                rasterizer.FillTriangle(t.v0, t.v1, t.v2);
                break;
            case Shading::Flat:
                rasterizer.FillTriangle(t.v0.position, t.v1.position, t.v2.position, Average(t));
                break;
            case Shading::Weights:
                // Pure red, green and blue: the color of a pixel is its barycentric coordinates.
                rasterizer.FillTriangle({ t.v0.position, rast::Color::Red }, { t.v1.position, rast::Color::Lime },
                                        { t.v2.position, rast::Color::Blue });
                break;
            }
        }

        window->Present(surface);
    }

    return 0;
}
