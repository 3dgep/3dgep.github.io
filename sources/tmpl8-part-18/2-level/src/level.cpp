#include "level.hpp"

#include <algorithm>
#include <fstream>

namespace Tmpl8
{
    void Level::Load(const std::string& file)
    {
        std::ifstream in(file);
        if (!in)
            NotifyUser("Can't open " + file);
        rows.clear();
        std::string line;
        while (std::getline(in, line))
            rows.push_back(line);

        // Make all rows equally long, so that every row has a tile in every column.
        columns = 0;
        for (const std::string& row : rows)
            columns = std::max(columns, static_cast<int>(row.size()));
        for (std::string& row : rows)
            row.resize(columns, ' ');
    }

    char Level::TileAt(int x, int y) const
    {
        if (x < 0 || x >= columns || y < 0 || y >= static_cast<int>(rows.size()))
            return ' ';
        return rows[y][x];
    }

    void Level::Draw(Surface& screen)
    {
        for (int y = 0; y < static_cast<int>(rows.size()); y++)
        {
            for (int x = 0; x < columns; x++)
            {
                const int frame = Frame(x, y);
                if (frame < 0)
                    continue;
                tiles.SetFrame(frame);
                tiles.Draw(screen, x * TileSize, y * TileSize);
            }
        }
    }

    // The frame in tiles.png for a tile, or -1 for an empty tile.
    int Level::Frame(int x, int y) const
    {
        switch (TileAt(x, y))
        {
        case '#': case '*': case '~': return GroundFrame(x, y);
        case 'B': return 26;         // a crate
        case 'C': return 151;        // a coin
        case '^': return 68;         // spikes
        case 'F': return 111;        // the flag
        case '|': return 131;        // the pole of the flag
        case 'g': return 124;        // grass
        case 't': return 126;        // a tree
        case 'm': return 128;        // a mushroom
        case 'c': return 127;        // a cactus
        case 's': return 145;        // a snowman
        default: return -1;
        }
    }

    // Ground tiles fit together: the frame depends on which neighbours are ground too.
    int Level::GroundFrame(int x, int y) const
    {
        // The column of the frame in tiles.png: 0 alone, 1 the left end, 2 the middle, 3 the right end.
        const bool left = IsGround(x - 1, y), right = IsGround(x + 1, y);
        const int column = left ? (right ? 2 : 3) : (right ? 1 : 0);
        // The row: the top of the ground (grass, sand or snow), or the dirt below it, with an edge at the bottom.
        const bool above = IsGround(x, y - 1), below = IsGround(x, y + 1);
        const char tile = TileAt(x, y);
        const int top = tile == '~' ? 2 : tile == '*' ? 4 : 0; // sand is 2 rows below grass, snow 4
        const int row = above ? (below ? 6 : 7) : top + (below ? 1 : 0);
        return row * 20 + column;
    }

    bool Level::IsGround(int x, int y) const
    {
        // Outside the level, at the sides and below it, the ground goes on.
        if (x < 0 || x >= columns || y >= static_cast<int>(rows.size()))
            return true;
        const char tile = TileAt(x, y);
        return tile == '#' || tile == '*' || tile == '~';
    }
}
