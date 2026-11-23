#pragma once

#include "surface.hpp"

#include <string_view>

namespace Tmpl8
{
    // Text in a pixel font: a sprite with a frame for every character from ' ' to '~'.
    class Text
    {
    public:
        void Print(Surface& screen, std::string_view text, int x, int y);
        void Centre(Surface& screen, std::string_view text, int y);
        static int Width(std::string_view text) { return static_cast<int>(text.size()) * Advance; }

        static constexpr int Advance = 7; // the distance from one character to the next, in pixels
        static constexpr int Height = 9;

    private:
        Sprite font{ "assets/font.png", 16, 6 };
    };
}
