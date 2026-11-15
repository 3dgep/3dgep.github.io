#include "game.hpp"

#include <SDL3/SDL_keyboard.h>

#include <format>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

namespace Tmpl8
{
    constexpr int TileSize = 64;    // the tiles are 64 x 64 pixels
    constexpr int SheetColumns = 10; // terrain.png has 10 tiles on each row
    constexpr int MapWidth = 12, MapHeight = 8;

    Surface terrain("assets/terrain.png");
    Sprite objectSprites("assets/objects.png", 8, 2);
    Sprite tank("assets/tank.png", 16), barrel("assets/barrel.png", 16);

    int ground[MapHeight][MapWidth] = {
        {  0,  0, 10,  1,  0,  0, 10,  0,  0,  0, 10,  0 },
        {  2,  2,  2, 12,  2,  2,  2, 14,  0,  0,  0,  0 },
        {  0, 10,  0,  1,  0,  0,  0,  1,  0, 10,  0,  0 },
        {  0,  0,  0,  1,  0, 10,  0,  1,  0,  0,  0, 10 },
        { 18, 18, 18, 38, 18, 18, 18, 38, 18, 18, 18, 18 },
        { 20, 30, 20, 21, 20, 20, 20, 21, 20, 30, 20, 20 },
        { 22, 22, 22, 25, 22, 22, 22, 25, 22, 22, 22, 22 },
        { 20, 20, 30, 20, 20, 30, 20, 20, 20, 20, 30, 20 }
    };

    int objects[MapHeight][MapWidth] = {
        { -1,  0, -1, -1, -1,  1, -1, -1,  2, -1,  0, -1 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  1 },
        {  0, -1, 14, -1, -1,  3, -1, -1, -1, -1,  2, -1 },
        { -1,  1, -1, -1, -1, -1, -1, -1,  5,  5, -1,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        {  7, -1, -1, -1, -1,  4, -1, -1, -1, 11, -1,  3 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, -1, -1 },
        { -1, -1, 10, 10, -1, -1,  8, -1, -1, -1,  7, -1 }
    };

    // Which objects block the tank: trees, crates, sandbags, barrels and barricades do; oil and twigs don't.
    bool solid[16] = {
        true, true, true, true, true, true, true, true,
        true, true, true, true, false, false, false, false
    };

    float tankX = 3 * TileSize + 32, tankY = 2 * TileSize + 32; // the middle of the tank
    int tankFrame = 0;

    bool editing = false;     // E switches between driving and editing
    int layer = 0;            // the layer that you edit: 0 is the ground, 1 the objects
    int brush[2] = { 0, -1 }; // the tile that you place, for each layer (-1: no object)
    std::string message;      // a message for a few seconds, after saving or loading
    float messageTime = 0;

    void DrawTile(Surface& target, int tile, int x, int y)
    {
        const int column = tile % SheetColumns, row = tile / SheetColumns;
        const Pixel* src = terrain.GetBuffer() + column * TileSize + row * TileSize * terrain.GetPitch();
        Pixel* dst = target.GetBuffer() + x + y * target.GetPitch();
        for (int i = 0; i < TileSize; i++)
        {
            for (int j = 0; j < TileSize; j++)
                dst[j] = src[j];
            src += terrain.GetPitch();
            dst += target.GetPitch();
        }
    }

    // Can the tank go here? (x, y) is a point in the map, in pixels.
    bool IsFree(float x, float y)
    {
        if (x < 0 || y < 0 || x >= MapWidth * TileSize || y >= MapHeight * TileSize)
            return false; // outside the map
        const int object = objects[static_cast<int>(y) / TileSize][static_cast<int>(x) / TileSize];
        return object == -1 || !solid[object];
    }

    // Can the tank stand here? Checks the four corners of a 36 x 36 box around the middle of the tank.
    bool TankFits(float x, float y)
    {
        constexpr float r = 18;
        return IsFree(x - r, y - r) && IsFree(x + r, y - r) && IsFree(x - r, y + r) && IsFree(x + r, y + r);
    }

    // Writes one layer of the map to a file: a line of numbers for every row, separated by commas.
    void SaveLayer(std::ofstream& file, int layer[MapHeight][MapWidth])
    {
        for (int y = 0; y < MapHeight; y++)
            for (int x = 0; x < MapWidth; x++)
                file << std::format("{:3}", layer[y][x]) << (x < MapWidth - 1 ? "," : "\n");
    }

    // Saves the map to a text file: the size of the map, and then the two layers. Returns false when that fails.
    bool SaveMap(const std::string& filename)
    {
        std::ofstream file(filename);
        if (!file) return false; // the file couldn't be created
        file << MapWidth << " " << MapHeight << "\n\n";
        SaveLayer(file, ground);
        file << "\n"; // an empty line between the layers
        SaveLayer(file, objects);
        return static_cast<bool>(file); // false when something went wrong while writing
    }

    // Reads one layer from a file into layer. Returns false when a number can't be read.
    bool LoadLayer(std::ifstream& file, int layer[MapHeight][MapWidth])
    {
        for (int y = 0; y < MapHeight; y++)
            for (int x = 0; x < MapWidth; x++)
            {
                file >> layer[y][x];
                if (x < MapWidth - 1)
                {
                    char comma;
                    file >> comma;
                }
            }
        return static_cast<bool>(file); // false when a number couldn't be read
    }

    // Loads the map from a text file. The map only changes when the whole file is correct; otherwise, it returns false.
    bool LoadMap(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file) return false; // the file doesn't exist (or can't be opened)
        int width, height;
        file >> width >> height;
        if (!file || width != MapWidth || height != MapHeight) return false;
        int newGround[MapHeight][MapWidth], newObjects[MapHeight][MapWidth];
        if (!LoadLayer(file, newGround) || !LoadLayer(file, newObjects)) return false;
        for (int y = 0; y < MapHeight; y++)
            for (int x = 0; x < MapWidth; x++)
            {
                ground[y][x] = newGround[y][x];
                objects[y][x] = newObjects[y][x];
            }
        return true;
    }

    // The start of a binary map file: a "magic" code that says what kind of file it is, and the size of the map.
    struct MapHeader
    {
        char magic[4];
        int width, height;
    };

    // Saves the map to a binary file: the header, and then the bytes of the two layers, just as they are in memory.
    bool SaveMapBinary(const std::string& filename)
    {
        std::ofstream file(filename, std::ios::binary);
        if (!file) return false;
        const MapHeader header = { { 'T', 'M', 'A', 'P' }, MapWidth, MapHeight };
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        file.write(reinterpret_cast<const char*>(ground), sizeof(ground));
        file.write(reinterpret_cast<const char*>(objects), sizeof(objects));
        return static_cast<bool>(file);
    }

    // Loads the map from a binary file; like LoadMap, the map only changes when the whole file is correct.
    bool LoadMapBinary(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;
        MapHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!file || header.magic[0] != 'T' || header.magic[1] != 'M' || header.magic[2] != 'A' || header.magic[3] != 'P')
            return false; // not a map file
        if (header.width != MapWidth || header.height != MapHeight) return false;
        int newGround[MapHeight][MapWidth], newObjects[MapHeight][MapWidth];
        file.read(reinterpret_cast<char*>(newGround), sizeof(newGround));
        file.read(reinterpret_cast<char*>(newObjects), sizeof(newObjects));
        if (!file) return false;
        for (int y = 0; y < MapHeight; y++)
            for (int x = 0; x < MapWidth; x++)
            {
                ground[y][x] = newGround[y][x];
                objects[y][x] = newObjects[y][x];
            }
        return true;
    }

    // -----------------------------------------------------------
    // Initialize the application
    // -----------------------------------------------------------
    void Game::Init()
    {
        if (!LoadMap("assets/map.txt")) std::println("Could not load assets/map.txt: using the map in the code.");
    }

    // -----------------------------------------------------------
    // Close down application
    // -----------------------------------------------------------
    void Game::Shutdown()
    {
    }

    // The scancode of the key that types this letter on the player's keyboard: SDLK_S is the key with an "s" on it,
    // also when the keyboard isn't a US keyboard.
    SDL_Scancode KeyFor(SDL_Keycode key)
    {
        return SDL_GetScancodeFromKey(key, nullptr);
    }

    // Drives the tank with the arrow keys (or W, A, S and D); the tank stops at solid objects.
    void Game::Update(float deltaTime)
    {
        const float speed = 0.15f * deltaTime; // pixels per millisecond
        float newX = tankX, newY = tankY;
        if (GetKey(SDL_SCANCODE_UP) || GetKey(SDL_SCANCODE_W)) newY -= speed, tankFrame = 0;
        if (GetKey(SDL_SCANCODE_RIGHT) || GetKey(SDL_SCANCODE_D)) newX += speed, tankFrame = 4;
        if (GetKey(SDL_SCANCODE_DOWN) || GetKey(SDL_SCANCODE_S)) newY += speed, tankFrame = 8;
        if (GetKey(SDL_SCANCODE_LEFT) || GetKey(SDL_SCANCODE_A)) newX -= speed, tankFrame = 12;
        if (TankFits(newX, tankY)) tankX = newX;
        if (TankFits(tankX, newY)) tankY = newY;
    }

    // The map editor: Tab chooses the layer, Z and X the tile; the left mouse button places it, the right one picks
    // the tile under the mouse. S saves the map, L loads it again; with Shift, as a binary file.
    void Game::Edit(float deltaTime)
    {
        if (GetKeyPressed(SDL_SCANCODE_TAB)) layer = 1 - layer;
        const int lowest = layer == 0 ? 0 : -1, highest = layer == 0 ? 39 : 15;
        if (GetKeyPressed(SDL_SCANCODE_Z)) brush[layer] = brush[layer] == lowest ? highest : brush[layer] - 1;
        if (GetKeyPressed(SDL_SCANCODE_X)) brush[layer] = brush[layer] == highest ? lowest : brush[layer] + 1;
        const int tx = mousex / TileSize, ty = mousey / TileSize; // the tile under the mouse
        if (tx >= 0 && tx < MapWidth && ty >= 0 && ty < MapHeight)
        {
            int& tile = layer == 0 ? ground[ty][tx] : objects[ty][tx];
            if (mouseLeft) tile = brush[layer];
            if (mouseRight) brush[layer] = tile;
            // the tile that you would place, with a white frame around it
            if (layer == 0) DrawTile(screen, brush[0], tx * TileSize, ty * TileSize);
            else if (brush[1] != -1)
            {
                objectSprites.SetFrame(brush[1]);
                objectSprites.Draw(screen, tx * TileSize, ty * TileSize);
            }
            screen.Box(tx * TileSize, ty * TileSize, tx * TileSize + TileSize - 1, ty * TileSize + TileSize - 1, 0xffffff);
        }

        const bool shift = GetKey(SDL_SCANCODE_LSHIFT) || GetKey(SDL_SCANCODE_RSHIFT);
        if (GetKeyPressed(KeyFor(SDLK_S)) && !shift)
            message = SaveMap("assets/map.txt") ? "SAVED ASSETS/MAP.TXT" : "COULD NOT SAVE ASSETS/MAP.TXT";
        if (GetKeyPressed(KeyFor(SDLK_L)) && !shift)
            message = LoadMap("assets/map.txt") ? "LOADED ASSETS/MAP.TXT" : "COULD NOT LOAD ASSETS/MAP.TXT";
        if (GetKeyPressed(KeyFor(SDLK_S)) && shift)
            message = SaveMapBinary("assets/map.bin") ? "SAVED ASSETS/MAP.BIN" : "COULD NOT SAVE ASSETS/MAP.BIN";
        if (GetKeyPressed(KeyFor(SDLK_L)) && shift)
            message = LoadMapBinary("assets/map.bin") ? "LOADED ASSETS/MAP.BIN" : "COULD NOT LOAD ASSETS/MAP.BIN";
        if (GetKeyPressed(KeyFor(SDLK_S)) || GetKeyPressed(KeyFor(SDLK_L))) messageTime = 3000;

        // what you are doing, at the top of the screen
        screen.Bar(0, 0, ScreenWidth - 1, 15, 0x202020);
        screen.Print(std::format("EDIT {}: TILE {}   TAB: LAYER   Z/X: TILE   RIGHT CLICK: PICK   S/L: SAVE/LOAD (SHIFT: BINARY)   E: DRIVE",
            layer == 0 ? "GROUND" : "OBJECTS", brush[layer]), 4, 5, 0xffffff);
        if (messageTime > 0)
        {
            messageTime -= deltaTime;
            screen.Bar(0, 16, ScreenWidth - 1, 31, 0x202020);
            screen.Print(message, 4, 21, 0xffff00);
        }
    }

    // -----------------------------------------------------------
    // Main application tick function
    // -----------------------------------------------------------
    void Game::Tick(float deltaTime)
    {
        screen.Clear(0);
        for (int y = 0; y < MapHeight; y++)
            for (int x = 0; x < MapWidth; x++)
            {
                DrawTile(screen, ground[y][x], x * TileSize, y * TileSize);
                if (objects[y][x] != -1)
                {
                    objectSprites.SetFrame(objects[y][x]);
                    objectSprites.Draw(screen, x * TileSize, y * TileSize);
                }
            }

        tank.SetFrame(tankFrame);
        barrel.SetFrame(tankFrame);
        tank.Draw(screen, static_cast<int>(tankX) - 32, static_cast<int>(tankY) - 32);
        barrel.Draw(screen, static_cast<int>(tankX) - 32, static_cast<int>(tankY) - 32);

        // E switches between driving and editing
        if (GetKeyPressed(KeyFor(SDLK_E))) editing = !editing;
        if (editing) Edit(deltaTime);
        else Update(deltaTime);
    }
}
