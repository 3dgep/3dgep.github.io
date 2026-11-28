#include <rast/Rasterizer.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <optional>
#include <utility>

namespace rast
{

Rasterizer::Rasterizer(Surface& colorTarget)
    : m_ColorTarget{ &colorTarget }
{}

void Rasterizer::SetColorTarget(Surface& colorTarget)
{
    m_ColorTarget = &colorTarget;
}

void Rasterizer::SetScissor(const Rect& scissor)
{
    m_Scissor = scissor;
}

void Rasterizer::ResetScissor()
{
    m_Scissor.reset();
}

Rect Rasterizer::ClipRect() const
{
    const Rect bounds = m_ColorTarget->Bounds();
    return m_Scissor ? Intersect(*m_Scissor, bounds) : bounds;
}

void Rasterizer::Clear(Color color)
{
    // Without a scissor, the whole surface is cleared, and Surface::Clear does that fastest.
    if (ClipRect() == m_ColorTarget->Bounds())
    {
        m_ColorTarget->Clear(color);
    }
    else
    {
        FillRect(ClipRect(), color);
    }
}

void Rasterizer::SetPixel(Point point, Color color)
{
    if (ClipRect().Contains(point))
    {
        m_ColorTarget->Pixels()[point.x, point.y] = color;
    }
}

void Rasterizer::FillRect(const Rect& rect, Color color)
{
    // Clip once, so that the loops below never have to check a pixel.
    const Rect clipped = Intersect(rect, ClipRect());
    if (clipped.IsEmpty())
    {
        return;
    }

    auto pixels = m_ColorTarget->Pixels();
    for (int y = clipped.top; y < clipped.bottom; ++y)
    {
        // The pixels of one row are next to each other in memory.
        std::fill_n(&pixels[clipped.left, y], clipped.Width(), color);
    }
}

void Rasterizer::DrawRect(const Rect& rect, Color color)
{
    if (rect.IsEmpty())
    {
        return;
    }

    // The top and bottom rows, then the left and right columns between them.
    FillRect({ rect.left, rect.top, rect.right, rect.top + 1 }, color);
    FillRect({ rect.left, rect.bottom - 1, rect.right, rect.bottom }, color);
    FillRect({ rect.left, rect.top + 1, rect.left + 1, rect.bottom - 1 }, color);
    FillRect({ rect.right - 1, rect.top + 1, rect.right, rect.bottom - 1 }, color);
}

namespace
{

// a / b, rounded up instead of towards zero. b must be positive.
std::int64_t CeilDiv(std::int64_t a, std::int64_t b)
{
    return a >= 0 ? (a + b - 1) / b : -(-a / b);
}

// The half-open range of offsets [first, last) that moves start into [lo, hi),
// when the coordinate is start + sign * offset.
struct OffsetRange
{
    std::int64_t first;
    std::int64_t last;
};

OffsetRange OffsetsInside(int start, int sign, int lo, int hi)
{
    if (sign > 0)
    {
        return { std::int64_t{ lo } - start, std::int64_t{ hi } - start };
    }
    return { std::int64_t{ start } - hi + 1, std::int64_t{ start } - lo + 1 };
}

// At step i, Bresenham's minor offset is k(i) = floor((2 * i * dMinor + dMajor) / (2 * dMajor)).
// This is the first step at which k(i) >= k.
std::int64_t FirstStepWithOffset(std::int64_t k, std::int64_t dMajor, std::int64_t dMinor)
{
    if (k <= 0)
    {
        return 0;
    }
    if (dMinor == 0)
    {
        return std::numeric_limits<std::int64_t>::max(); // The minor offset is always 0.
    }
    return CeilDiv(2 * dMajor * k - dMajor, 2 * dMinor);
}

} // namespace

void Rasterizer::DrawLine(Point p0, Point p1, Color color)
{
    // Describe the line along its major axis (the one with the larger distance), and its minor axis.
    const int dx = p1.x - p0.x;
    const int dy = p1.y - p0.y;
    const bool xMajor = std::abs(dx) >= std::abs(dy);

    const int majorStart = xMajor ? p0.x : p0.y;
    const int minorStart = xMajor ? p0.y : p0.x;
    const std::int64_t dMajor = std::abs(xMajor ? dx : dy);
    const std::int64_t dMinor = std::abs(xMajor ? dy : dx);
    const int majorSign = (xMajor ? dx : dy) < 0 ? -1 : 1;
    const int minorSign = (xMajor ? dy : dx) < 0 ? -1 : 1;

    // The line has one pixel per step along the major axis: steps 0 to dMajor - 1.
    // p1 would be step dMajor, which is not drawn.
    if (dMajor == 0)
    {
        return;
    }

    // Clip: keep only the steps whose pixel is inside the clip rectangle.
    const Rect clip = ClipRect();
    const OffsetRange majorSteps = xMajor ? OffsetsInside(majorStart, majorSign, clip.left, clip.right)
                                          : OffsetsInside(majorStart, majorSign, clip.top, clip.bottom);
    const OffsetRange minorOffsets = xMajor ? OffsetsInside(minorStart, minorSign, clip.top, clip.bottom)
                                            : OffsetsInside(minorStart, minorSign, clip.left, clip.right);

    const std::int64_t first = std::max({ std::int64_t{ 0 }, majorSteps.first,
                                          FirstStepWithOffset(minorOffsets.first, dMajor, dMinor) });
    const std::int64_t last = std::min({ dMajor, majorSteps.last,
                                         FirstStepWithOffset(minorOffsets.last, dMajor, dMinor) });
    if (first >= last)
    {
        return;
    }

    // Start at the first visible step, with the minor offset and the error that Bresenham
    // would have there, so the clipped line has exactly the same pixels as the whole line.
    const std::int64_t k = (2 * first * dMinor + dMajor) / (2 * dMajor);
    std::int64_t error = 2 * first * dMinor + dMajor - 2 * dMajor * (k + 1);
    int major = majorStart + majorSign * static_cast<int>(first);
    int minor = minorStart + minorSign * static_cast<int>(k);

    auto pixels = m_ColorTarget->Pixels();
    for (std::int64_t step = first; step < last; ++step)
    {
        if (xMajor)
        {
            pixels[major, minor] = color;
        }
        else
        {
            pixels[minor, major] = color;
        }

        // One step along the major axis. When the error becomes 0 or more,
        // the line has moved half a pixel or more along the minor axis.
        major += majorSign;
        error += 2 * dMinor;
        if (error >= 0)
        {
            minor += minorSign;
            error -= 2 * dMajor;
        }
    }
}

namespace
{

// The bounding box of a circle: the smallest Rect that contains all of its pixels.
Rect CircleBounds(Point center, int radius)
{
    return Rect::FromSize(center.x - radius, center.y - radius, 2 * radius, 2 * radius);
}

// Walks one eighth of the circle's outline with the midpoint algorithm, and calls plot(x, y)
// for that pixel and its mirror images in the seven other eighths.
//
// The pixel a columns right of the center and b rows below it has its center at
// (a + 0.5, b + 0.5) from the circle's center. It's inside the circle when
// (a + 0.5)^2 + (b + 0.5)^2 < r^2, which is (2a + 1)^2 + (2b + 1)^2 < 4r^2 in integers.
template <typename Plot>
void WalkCircleOutline(Point center, int radius, Plot plot)
{
    // Mirroring a pixel offset a across the center gives the offset -1 - a, so the pixel
    // a columns right of the center mirrors to the pixel a + 1 columns left of it.
    auto plotMirrored = [&](int a, int b) {
        plot(center.x + a, center.y + b);
        plot(center.x - 1 - a, center.y + b);
        plot(center.x + a, center.y - 1 - b);
        plot(center.x - 1 - a, center.y - 1 - b);
    };

    // Start at the bottom of the circle (a = 0, b = r - 1) and walk right until the diagonal (a = b).
    int a = 0;
    int b = radius - 1;
    // d = (2a + 1)^2 + (2b + 1)^2 - 4r^2: negative while the pixel (a, b) is inside the circle.
    std::int64_t d = 2 - 4 * std::int64_t{ radius };

    while (a <= b)
    {
        plotMirrored(a, b);
        if (a != b)
        {
            plotMirrored(b, a); // The mirror image across the diagonal (unless it's the same pixel).
        }

        // One column to the right: (2a + 3)^2 - (2a + 1)^2 = 8(a + 1).
        ++a;
        d += 8 * std::int64_t{ a };

        // If that pixel is outside the circle, take the one below it: (2b - 1)^2 - (2b + 1)^2 = -8b.
        if (d >= 0)
        {
            d -= 8 * std::int64_t{ b };
            --b;
        }
    }
}

} // namespace

void Rasterizer::DrawCircle(Point center, int radius, Color color)
{
    if (radius <= 0)
    {
        return;
    }

    // Clip with the bounding box: draw nothing, draw without checks, or check every pixel.
    const Rect bounds = CircleBounds(center, radius);
    const Rect clip = ClipRect();
    const Rect visible = Intersect(bounds, clip);
    if (visible.IsEmpty())
    {
        return;
    }

    auto pixels = m_ColorTarget->Pixels();
    if (visible == bounds)
    {
        WalkCircleOutline(center, radius, [&](int x, int y) { pixels[x, y] = color; });
    }
    else
    {
        WalkCircleOutline(center, radius, [&](int x, int y) {
            if (clip.Contains({ x, y }))
            {
                pixels[x, y] = color;
            }
        });
    }
}

void Rasterizer::FillCircle(Point center, int radius, Color color)
{
    if (radius <= 0 || Intersect(CircleBounds(center, radius), ClipRect()).IsEmpty())
    {
        return;
    }

    // Walk down from the middle row (b = 0) to the bottom row (b = r - 1). Each row is a span
    // from w + 1 pixels left of the center to w pixels right of it, where w is the offset of
    // the outermost pixel inside the circle. The same span is mirrored to the row above the center.
    int w = radius - 1;
    // d = (2w + 1)^2 + (2b + 1)^2 - 4r^2 for the pixel (w, b), with b = 0.
    std::int64_t d = 2 - 4 * std::int64_t{ radius };

    for (int b = 0; b < radius; ++b)
    {
        // Move w inwards until the pixel (w, b) is inside the circle: (2w - 1)^2 - (2w + 1)^2 = -8w.
        while (d >= 0)
        {
            d -= 8 * std::int64_t{ w };
            --w;
        }

        // FillRect clips each span, so a span that's outside the clip rectangle costs almost nothing.
        FillRect({ center.x - 1 - w, center.y + b, center.x + w + 1, center.y + b + 1 }, color);
        FillRect({ center.x - 1 - w, center.y - 1 - b, center.x + w + 1, center.y - b }, color);

        // One row down: (2b + 3)^2 - (2b + 1)^2 = 8(b + 1).
        d += 8 * std::int64_t{ b + 1 };
    }
}

namespace
{

// Vertices are snapped to fixed-point numbers with 8 bits for the fraction: 1/256 of a pixel.
constexpr int SubpixelBits = 8;
constexpr std::int64_t SubpixelScale = std::int64_t{ 1 } << SubpixelBits; // 256: one pixel.
constexpr std::int64_t HalfPixel = SubpixelScale / 2;                     // 128: half a pixel.

struct FixedPoint
{
    std::int64_t x;
    std::int64_t y;
};

FixedPoint Snap(Vec2 v)
{
    constexpr float scale = static_cast<float>(SubpixelScale);
    return { std::llround(v.x * scale), std::llround(v.y * scale) };
}

// a / b, rounded down instead of towards zero. b must be positive.
std::int64_t FloorDiv(std::int64_t a, std::int64_t b)
{
    return a >= 0 ? a / b : -CeilDiv(-a, b);
}

// The edge function of the edge from a to b: positive for points on its right (as seen on the
// screen, looking from a to b), zero on the edge, negative on its left. In fixed-point units.
std::int64_t EdgeFunction(FixedPoint a, FixedPoint b, FixedPoint p)
{
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

// An edge function, ready to be stepped from pixel to pixel.
struct Edge
{
    std::int64_t value; // At the center of the first pixel, with the fill rule's bias.
    std::int64_t stepX; // Added for every pixel to the right.
    std::int64_t stepY; // Added for every pixel down.
    std::int64_t bias;  // The fill rule's bias in value: 0 or -1.
};

// The edge from a to b, of a triangle whose vertices go clockwise on the screen, starting at the
// pixel center `start`. The top-left rule: a pixel center exactly on the edge belongs to the
// triangle only when the edge is a top edge or a left edge.
Edge MakeEdge(FixedPoint a, FixedPoint b, FixedPoint start)
{
    const std::int64_t dx = b.x - a.x;
    const std::int64_t dy = b.y - a.y;

    // Going clockwise on the screen, a top edge goes exactly to the right,
    // and a left edge goes up.
    const bool topEdge = dy == 0 && dx > 0;
    const bool leftEdge = dy < 0;
    const std::int64_t bias = topEdge || leftEdge ? 0 : -1;

    return { EdgeFunction(a, b, start) + bias, -dy * SubpixelScale, dx * SubpixelScale, bias };
}

// Everything that FillTriangle needs to know about a triangle before it visits the pixels.
struct TriangleSetup
{
    Rect box;          // The pixels to visit: the bounding box, clipped to the clip rectangle.
    Edge e0;           // Edge i is the edge opposite vertex i,
    Edge e1;           // starting at the center of the box's top-left pixel.
    Edge e2;
    std::int64_t area; // Twice the area, in fixed-point units. Always positive here.
    bool swapped;      // Whether v1 and v2 were swapped to make the vertices go clockwise.
};

// Snaps the vertices, culls the triangle, makes its vertices go clockwise, and clips its
// bounding box. Returns nothing when there are no pixels to visit.
std::optional<TriangleSetup> SetupTriangle(Vec2 v0, Vec2 v1, Vec2 v2, const Rect& clip, FrontFace frontFace,
                                           CullMode cullMode)
{
    FixedPoint p0 = Snap(v0);
    FixedPoint p1 = Snap(v1);
    FixedPoint p2 = Snap(v2);

    // Twice the area of the triangle. Positive when its vertices go clockwise on the screen.
    std::int64_t area = EdgeFunction(p0, p1, p2);
    if (area == 0)
    {
        return std::nullopt; // The vertices are on one line: the triangle has no inside.
    }

    // Front-facing or back-facing, and culled or not.
    const bool clockwise = area > 0;
    const bool frontFacing = clockwise == (frontFace == FrontFace::Clockwise);
    if ((cullMode == CullMode::Front && frontFacing) || (cullMode == CullMode::Back && !frontFacing))
    {
        return std::nullopt;
    }

    // From here on, the vertices always go clockwise.
    if (!clockwise)
    {
        std::swap(p1, p2);
        area = -area;
    }

    // The bounding box of the triangle, in whole pixels, clipped to the clip rectangle.
    const Rect bounds{ static_cast<int>(FloorDiv(std::min({ p0.x, p1.x, p2.x }), SubpixelScale)),
                       static_cast<int>(FloorDiv(std::min({ p0.y, p1.y, p2.y }), SubpixelScale)),
                       static_cast<int>(CeilDiv(std::max({ p0.x, p1.x, p2.x }), SubpixelScale)),
                       static_cast<int>(CeilDiv(std::max({ p0.y, p1.y, p2.y }), SubpixelScale)) };
    const Rect box = Intersect(bounds, clip);
    if (box.IsEmpty())
    {
        return std::nullopt;
    }

    // Edge i is the edge opposite vertex i. Each one starts at the center of the box's top-left pixel.
    const FixedPoint start{ box.left * SubpixelScale + HalfPixel, box.top * SubpixelScale + HalfPixel };
    return TriangleSetup{ box, MakeEdge(p1, p2, start), MakeEdge(p2, p0, start), MakeEdge(p0, p1, start), area,
                          !clockwise };
}

// Visits every pixel of the setup's box, and writes shade(w0, w1, w2) to the pixels whose
// center is inside the triangle. w0, w1 and w2 are the edge functions at the pixel center,
// without the fill rule's bias, so they are exactly proportional to the barycentric coordinates.
template <typename Shade>
void RasterizeTriangle(TriangleSetup setup, PixelView pixels, Shade shade)
{
    Edge& e0 = setup.e0;
    Edge& e1 = setup.e1;
    Edge& e2 = setup.e2;

    for (int y = setup.box.top; y < setup.box.bottom; ++y)
    {
        // The values at the first pixel of this row.
        std::int64_t w0 = e0.value;
        std::int64_t w1 = e1.value;
        std::int64_t w2 = e2.value;

        for (int x = setup.box.left; x < setup.box.right; ++x)
        {
            // The pixel center is inside when it's on the inside of all three edges.
            if (w0 >= 0 && w1 >= 0 && w2 >= 0)
            {
                pixels[x, y] = shade(w0 - e0.bias, w1 - e1.bias, w2 - e2.bias);
            }

            w0 += e0.stepX;
            w1 += e1.stepX;
            w2 += e2.stepX;
        }

        e0.value += e0.stepY;
        e1.value += e1.stepY;
        e2.value += e2.stepY;
    }
}

// The color of a pixel with the barycentric coordinates l0, l1 and l2: the weighted sum of the
// three vertex colors, channel by channel, rounded to the nearest whole number.
Color Interpolate(Color c0, Color c1, Color c2, float l0, float l1, float l2)
{
    auto channel = [&](std::uint8_t a, std::uint8_t b, std::uint8_t c) {
        const float value = l0 * static_cast<float>(a) + l1 * static_cast<float>(b) + l2 * static_cast<float>(c);
        return static_cast<std::uint8_t>(std::clamp(value, 0.0f, 255.0f) + 0.5f);
    };
    return Color{ channel(c0.R(), c1.R(), c2.R()), channel(c0.G(), c1.G(), c2.G()),
                  channel(c0.B(), c1.B(), c2.B()), channel(c0.A(), c1.A(), c2.A()) };
}

} // namespace

void Rasterizer::FillTriangle(Vec2 v0, Vec2 v1, Vec2 v2, Color color)
{
    const auto setup = SetupTriangle(v0, v1, v2, ClipRect(), m_FrontFace, m_CullMode);
    if (!setup)
    {
        return;
    }

    // Every pixel gets the same color, so the shading ignores the edge functions.
    RasterizeTriangle(*setup, m_ColorTarget->Pixels(), [=](std::int64_t, std::int64_t, std::int64_t) { return color; });
}

void Rasterizer::FillTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
    const auto setup = SetupTriangle(v0.position, v1.position, v2.position, ClipRect(), m_FrontFace, m_CullMode);
    if (!setup)
    {
        return;
    }

    // The colors in the same order as the setup's vertices, which may have swapped v1 and v2.
    const Color c0 = v0.color;
    const Color c1 = setup->swapped ? v2.color : v1.color;
    const Color c2 = setup->swapped ? v1.color : v2.color;

    // The barycentric coordinate of vertex i is the edge function opposite it, divided by the area.
    const float inverseArea = 1.0f / static_cast<float>(setup->area);
    RasterizeTriangle(*setup, m_ColorTarget->Pixels(), [=](std::int64_t w0, std::int64_t w1, std::int64_t w2) {
        const float l0 = static_cast<float>(w0) * inverseArea;
        const float l1 = static_cast<float>(w1) * inverseArea;
        const float l2 = static_cast<float>(w2) * inverseArea;
        return Interpolate(c0, c1, c2, l0, l1, l2);
    });
}

} // namespace rast
