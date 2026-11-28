// Part 1: The Framebuffer
// Fills a Surface with an animated gradient and shows it in a window.

#include <rast/Surface.hpp>
#include <rast/Window.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numbers>
#include <print>

// Converts a value from [0, 1] to a color channel in [0, 255].
std::uint8_t ToByte(float value)
{
    return static_cast<std::uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
}

int main()
{
    constexpr int width = 640;
    constexpr int height = 360;

    auto window = rast::Window::Create("Part 1: The Framebuffer", width * 2, height * 2);
    if (!window)
    {
        std::println(stderr, "{}", window.error());
        return 1;
    }

    rast::Surface surface{ width, height };

    const auto start = std::chrono::steady_clock::now();

    while (window->PollEvents())
    {
        const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();

        auto pixels = surface.Pixels();
        for (int y = 0; y < surface.Height(); ++y)
        {
            for (int x = 0; x < surface.Width(); ++x)
            {
                // u and v go from 0 at the top-left corner to 1 at the bottom-right corner.
                const float u = static_cast<float>(x) / static_cast<float>(width - 1);
                const float v = static_cast<float>(y) / static_cast<float>(height - 1);

                const float red = u;
                const float green = v;
                const float blue = 0.5f + 0.5f * std::sin(2.0f * time - (u + v) * std::numbers::pi_v<float>);

                pixels[y, x] = rast::Color{ ToByte(red), ToByte(green), ToByte(blue) };
            }
        }

        window->Present(surface);
    }

    return 0;
}
