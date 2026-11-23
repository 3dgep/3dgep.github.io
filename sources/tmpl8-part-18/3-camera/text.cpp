#include "text.hpp"

namespace Tmpl8
{
    void Text::Print(Surface& screen, std::string_view text, int x, int y)
    {
        for (const char character : text)
        {
            if (character > ' ' && character <= '~') // a space has nothing to draw
            {
                font.SetFrame(character - ' ');
                font.Draw(screen, x, y);
            }
            x += Advance;
        }
    }

    void Text::Centre(Surface& screen, std::string_view text, int y)
    {
        Print(screen, text, (screen.GetWidth() - Width(text)) / 2, y);
    }
}
