#include "background.hpp"

#include "template.hpp"

namespace Tmpl8
{
    Background::Background(const std::string& file) : strip(file, 1), sky(strip.GetBuffer()[0] & 0xffffff)
    {
    }

    void Background::Draw(Surface& screen, int cameraX, int height)
    {
        screen.Clear(sky);
        // The strip repeats: start with the copy that sticks out over the left edge of the screen.
        const int width = strip.GetWidth();
        const int offset = static_cast<int>(cameraX * Parallax) % width;
        const int y = ScreenHeight - strip.GetHeight() + static_cast<int>(height * Parallax);
        for (int x = -offset; x < ScreenWidth; x += width)
        {
            strip.Draw(screen, x, y);
        }
    }
}
