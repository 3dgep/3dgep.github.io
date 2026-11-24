#pragma once

namespace Tmpl8
{
    // What the player chose in the settings. They are saved in settings.txt, so the game remembers them.
    struct Settings
    {
        int musicVolume = 70;    // in percent
        int effectsVolume = 100; // in percent
        bool fullscreen = false;

        void Load();
        void Save() const;
    };
}
