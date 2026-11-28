#pragma once

#include "Color.hpp"
#include "Rect.hpp"
#include "Surface.hpp"
#include "Vec2.hpp"
#include "Vertex.hpp"

#include <optional>

namespace rast
{

// The winding of a triangle is the direction in which its vertices v0, v1, v2 go around it,
// as seen on the screen: in pixel coordinates, where the y-axis points down. The front face
// says which winding makes a triangle front-facing; triangles with the other winding are back-facing.
enum class FrontFace
{
    Clockwise,        // Front-facing when v0, v1, v2 go clockwise on the screen (the default, like Direct3D).
    CounterClockwise, // Front-facing when v0, v1, v2 go counter-clockwise on the screen.
};

// Which triangles FillTriangle skips.
enum class CullMode
{
    None,  // Draw every triangle (the default).
    Front, // Skip the front-facing triangles.
    Back,  // Skip the back-facing triangles.
};

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

    // Which winding makes a triangle front-facing. FrontFace::Clockwise by default.
    void SetFrontFace(FrontFace frontFace) { m_FrontFace = frontFace; }
    FrontFace GetFrontFace() const { return m_FrontFace; }

    // Culling skips front-facing or back-facing triangles. It's off (CullMode::None) by default.
    void SetCullMode(CullMode cullMode) { m_CullMode = cullMode; }
    CullMode GetCullMode() const { return m_CullMode; }

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

    // Fills every pixel whose center is inside the triangle. The vertices are snapped to
    // 1/256 of a pixel. Pixel centers exactly on an edge follow the top-left rule, so triangles
    // that share an edge never leave a gap between them, and never draw a pixel twice.
    void FillTriangle(Vec2 v0, Vec2 v1, Vec2 v2, Color color);

    // Fills the triangle like the function above, but blends the colors of the three vertices
    // across it (Gouraud shading): every pixel gets the vertex colors, weighted by its
    // barycentric coordinates.
    void FillTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);

private:
    Surface* m_ColorTarget;
    std::optional<Rect> m_Scissor;
    FrontFace m_FrontFace = FrontFace::Clockwise;
    CullMode m_CullMode = CullMode::None;
};

} // namespace rast
