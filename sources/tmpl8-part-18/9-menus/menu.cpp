#include "menu.hpp"

#include "game.hpp"

namespace Tmpl8
{
    int Menu::Tick(Game& game)
    {
        const int count = static_cast<int>(labels.size());
        int choice = selected;
        if (game.GetKeyPressed(SDL_SCANCODE_DOWN))
            choice = (selected + 1) % count;
        if (game.GetKeyPressed(SDL_SCANCODE_UP))
            choice = (selected + count - 1) % count;
        // The mouse selects the button that it moves onto.
        const int item = ItemAt(game.MouseX(), game.MouseY());
        if (game.MouseMoved() && item != -1)
            choice = item;
        if (choice != selected)
        {
            selected = choice;
            game.GetSounds().Play(Effect::Select);
        }

        if (game.GetKeyPressed(SDL_SCANCODE_RETURN) || game.GetKeyPressed(SDL_SCANCODE_SPACE) ||
            (game.MouseClicked() && item == selected))
        {
            game.GetSounds().Play(Effect::Confirm);
            return selected;
        }
        return -1;
    }

    void Menu::Draw(Surface& screen, Text& text)
    {
        for (int i = 0; i < static_cast<int>(labels.size()); i++)
        {
            const int y = top + i * Spacing;
            button.SetFrame(i == selected ? 1 : 0);
            button.Draw(screen, Left(), y);
            text.Centre(screen, labels[i], y + (button.GetHeight() - Text::Height) / 2 + 1);
        }
    }

    int Menu::ItemAt(int x, int y) const
    {
        if (x < Left() || x >= Left() + button.GetWidth() || y < top)
            return -1;
        const int item = (y - top) / Spacing;
        if (item >= static_cast<int>(labels.size()) || (y - top) % Spacing >= button.GetHeight())
            return -1;
        return item;
    }
}
