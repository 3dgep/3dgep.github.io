#include "settings.hpp"

#include <fstream>

namespace Tmpl8
{
    void Settings::Load()
    {
        std::ifstream file("settings.txt");
        int music, effects;
        bool full;
        // Only use the values when all three could be read: the first time the game runs, there is no file yet.
        if (file >> music >> effects >> full)
        {
            musicVolume = music;
            effectsVolume = effects;
            fullscreen = full;
        }
    }

    void Settings::Save() const
    {
        std::ofstream file("settings.txt");
        file << musicVolume << ' ' << effectsVolume << ' ' << fullscreen << '\n';
    }
}
