#pragma once

#include <Audio/Audio.hpp>

#include <array>

namespace Tmpl8
{
    // The sound effects, in the same order as their files in Sounds::Sounds.
    enum class Effect { Jump, Coin, Stomp, Hurt, Select, Confirm, Win, Lose, Count };

    // All the sound effects and the music of the game.
    class Sounds
    {
    public:
        Sounds();
        void Play(Effect effect);
        void PlayMenuMusic() { PlayMusic(menuMusic); }
        void PlayLevelMusic() { PlayMusic(levelMusic); }
        void StopMusic();
        void SetEffectsVolume(int percent);
        void SetMusicVolume(int percent);

    private:
        void PlayMusic(Audio::Sound& music);

        // The sounds play through two groups, so the settings can change the volume of each group.
        Audio::Group effectsGroup;
        Audio::Group musicGroup;
        std::array<Audio::Sound, static_cast<int>(Effect::Count)> effects;
        Audio::Sound menuMusic;
        Audio::Sound levelMusic;
        Audio::Sound* currentMusic = nullptr; // the music that is playing, or nullptr
    };
}
