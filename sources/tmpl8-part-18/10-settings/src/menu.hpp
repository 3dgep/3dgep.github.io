#pragma once

#include "surface.hpp"
#include "template.hpp"

#include <string>
#include <vector>

namespace Tmpl8
{
    class Game;
    class Text;

    // A column of buttons. Choose one with the arrow keys and Enter, or click it.
    class Menu
    {
    public:
        explicit Menu(int top) : top(top) {}
        void Add(const std::string& label) { labels.push_back(label); }
        void SetLabel(int item, const std::string& label) { labels[item] = label; }
        int Selected() const { return selected; }
        int Tick(Game& game); // the item that was chosen in this frame, or -1
        void Draw(Surface& screen, Text& text);

    private:
        int ItemAt(int x, int y) const; // the item at a position on the screen, or -1
        int Left() const { return (ScreenWidth - button.GetWidth()) / 2; }

        static constexpr int Spacing = 28; // from the top of one button to the top of the next, in pixels
        Sprite button{ "assets/ui/button.png", 1, 2 }; // frame 0 is a button, frame 1 the selected button
        std::vector<std::string> labels;
        int top;          // the y of the first button
        int selected = 0;
    };
}
