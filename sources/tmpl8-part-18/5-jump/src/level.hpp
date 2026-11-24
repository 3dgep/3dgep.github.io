#pragma once

#include "body.hpp"
#include "surface.hpp"

#include <string>
#include <vector>

namespace Tmpl8
{
    class Camera;

    // A level: a grid of tiles, loaded from a text file with one character per tile.
    class Level
    {
    public:
        static constexpr int TileSize = 18; // in pixels
        static int ToTile(float pixels) { return static_cast<int>(std::floor(pixels / TileSize)); }

        void Load(const std::string& file);
        void Draw(Surface& screen, const Camera& camera);

        char TileAt(int x, int y) const;
        bool IsSolid(int x, int y) const;
        Body HitBox(int x, int y) const;
        void Move(Body& body, float deltaTime) const;

        float Width() const { return static_cast<float>(columns * TileSize); } // in pixels
        float Height() const { return static_cast<float>(rows.size() * TileSize); } // in pixels
        vec2 Size() const { return { Width(), Height() }; }
        vec2 PlayerStart() const { return playerStart; }

    private:
        bool IsGround(int x, int y) const;
        bool HitsSolid(const Body& body) const;
        int Frame(int x, int y) const;
        int GroundFrame(int x, int y) const;

        std::vector<std::string> rows; // the characters of the level, one string per row of tiles
        int columns = 0;
        vec2 playerStart;
        Sprite tiles{ "assets/tiles.png", 20, 9 };
    };
}
