#pragma once

namespace Tmpl8
{
    class Surface;

    // A screen of the game, like the main menu or a level. The game shows one scene at a time.
    class Scene
    {
    public:
        virtual ~Scene() = default;
        virtual void Tick(float deltaTime) = 0; // deltaTime is in seconds
        virtual void Draw(Surface& screen) = 0;
    };
}
