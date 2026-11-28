#pragma once

#include "Color.hpp"
#include "Rect.hpp"
#include "Surface.hpp"

#include <optional>

namespace rast
{

// Draws into a color target: the Surface that holds the colors of the pixels.
// The Rasterizer doesn't own the surface, so the surface must live at least
// as long as the Rasterizer draws into it.
class Rasterizer
{
public:
    explicit Rasterizer(Surface& colorTarget);

    // The surface that we draw into.
    void SetColorTarget(Surface& colorTarget);
    Surface& ColorTarget() const { return *m_ColorTarget; }

    // Only the pixels inside the scissor rectangle are changed.
    void SetScissor(const Rect& scissor);
    // Without a scissor rectangle, every pixel of the color target can be changed.
    void ResetScissor();

    // The pixels that drawing can change: the scissor rectangle, clipped to the color target.
    Rect ClipRect() const;

    // Sets every pixel inside the clip rectangle to the same color.
    void Clear(Color color);

    void SetPixel(Point point, Color color);

    // Fills every pixel inside the rectangle.
    void FillRect(const Rect& rect, Color color);

    // Draws the outline of the rectangle, one pixel wide, inside the rectangle.
    void DrawRect(const Rect& rect, Color color);

private:
    Surface* m_ColorTarget;
    std::optional<Rect> m_Scissor;
};

} // namespace rast
