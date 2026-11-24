#include "settings_scene.hpp"

#include "game.hpp"
#include "menu_scene.hpp"

#include <format>

namespace Tmpl8
{
    namespace
    {
        // The next volume, 10% up or down. Past 100% comes 0%, and the other way around.
        int NextVolume(int volume, int step)
        {
            return (volume + step * 10 + 110) % 110;
        }
    }

    SettingsScene::SettingsScene(Game& game) : game(game)
    {
        for (int i = 0; i < 4; i++)
            menu.Add("");
        UpdateLabels();
    }

    void SettingsScene::Tick(float deltaTime)
    {
        // Enter or a click goes to the next value, the left and right arrows go down and up.
        const int chosen = menu.Tick(game);
        int step = 0;
        if (game.GetKeyPressed(SDL_SCANCODE_LEFT))
            step = -1;
        if (game.GetKeyPressed(SDL_SCANCODE_RIGHT) || chosen != -1)
            step = 1;
        if (step == 0)
            return;

        Settings& settings = game.GetSettings();
        switch (menu.Selected())
        {
        case Music:
            settings.musicVolume = NextVolume(settings.musicVolume, step);
            game.GetSounds().SetMusicVolume(settings.musicVolume);
            break;
        case Effects:
            settings.effectsVolume = NextVolume(settings.effectsVolume, step);
            game.GetSounds().SetEffectsVolume(settings.effectsVolume);
            game.GetSounds().Play(Effect::Coin); // to hear how loud it is
            break;
        case Fullscreen:
            settings.fullscreen = !settings.fullscreen;
            SetFullscreen(settings.fullscreen);
            break;
        case Back:
            if (chosen == Back)
            {
                settings.Save();
                game.ChangeScene(std::make_unique<MenuScene>(game));
            }
            break;
        }
        UpdateLabels();
    }

    void SettingsScene::UpdateLabels()
    {
        const Settings& settings = game.GetSettings();
        menu.SetLabel(Music, std::format("MUSIC {:>3}%", settings.musicVolume));
        menu.SetLabel(Effects, std::format("EFFECTS {:>3}%", settings.effectsVolume));
        menu.SetLabel(Fullscreen, settings.fullscreen ? "FULLSCREEN ON" : "FULLSCREEN OFF");
        menu.SetLabel(Back, "BACK");
    }

    void SettingsScene::Draw(Surface& screen)
    {
        background.Draw(screen, 0);
        panel.Draw(screen, (ScreenWidth - panel.GetWidth()) / 2, 30);
        game.GetText().Centre(screen, "SETTINGS", 50);
        menu.Draw(screen, game.GetText());
    }
}
