#include "sounds.hpp"

#include <format>

namespace Tmpl8
{
    Sounds::Sounds()
    {
        const char* names[] = { "jump", "coin", "stomp", "hurt", "select", "confirm", "win", "lose" };
        for (int i = 0; i < static_cast<int>(effects.size()); i++)
        {
            effects[i].load(effectsGroup, std::format("assets/sounds/{}.ogg", names[i]));
        }
        menuMusic.load(musicGroup, "assets/music/menu.ogg", Audio::Sound::Type::Music);
        menuMusic.setLooping(true);
        levelMusic.load(musicGroup, "assets/music/level.ogg", Audio::Sound::Type::Music);
        levelMusic.setLooping(true);
    }

    void Sounds::Play(Effect effect)
    {
        // A one-shot is a copy of the sound that plays until it ends, so three coins in a row don't cut each other off.
        effects[static_cast<int>(effect)].playOneShot();
    }

    void Sounds::PlayMusic(Audio::Sound& music)
    {
        if (currentMusic == &music)
            return; // it is already playing
        StopMusic();
        currentMusic = &music;
        music.replay();
    }

    void Sounds::StopMusic()
    {
        if (currentMusic)
            currentMusic->stop();
        currentMusic = nullptr;
    }

    void Sounds::SetEffectsVolume(int percent)
    {
        effectsGroup.setVolume(percent / 100.0f);
    }

    void Sounds::SetMusicVolume(int percent)
    {
        musicGroup.setVolume(percent / 100.0f);
    }
}
