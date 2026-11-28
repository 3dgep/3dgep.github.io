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

    // Draws a line from p0 to p1 with Bresenham's algorithm. Like a Rect, the line is half-open:
    // p0 is drawn, but p1 is not, so lines that share an end point never draw it twice.
    void DrawLine(Point p0, Point p1, Color color);

    // A circle's center is the corner between four pixels, and a pixel is inside the circle when
    // its center is. So a circle with radius r is 2 * r pixels wide, like the Rect
    // Rect::FromSize(center.x - radius, center.y - radius, 2 * radius, 2 * radius).

    // Draws the outline of the circle: the outermost pixels of FillCircle, one pixel wide.
    void DrawCircle(Point center, int radius, Color color);

    // Fills every pixel inside the circle, one horizontal span per row.
    void FillCircle(Point center, int radius, Color color);

private:
    Surface* m_ColorTarget;
    std::optional<Rect> m_Scissor;
};

} // namespace rast
