#pragma once

#include "surface.hpp"

#include <string>

namespace Tmpl8
{
    // The sky, with a strip of clouds and hills that scrolls slower than the level.
    class Background
    {
    public:
        // The strip repeats sideways. The colour of its top-left pixel is the colour of the sky above it.
        explicit Background(const std::string& file);
        // height: how far the camera is above the bottom of the level (the strip goes down as the camera goes up).
        void Draw(Surface& screen, int cameraX, int height = 0);

    private:
        static constexpr float Parallax = 0.3f; // the strip moves at 30% of the speed of the level
        Sprite strip;
        Pixel sky;
    };
}
