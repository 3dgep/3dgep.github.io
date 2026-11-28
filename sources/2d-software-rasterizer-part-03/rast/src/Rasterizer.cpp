#include <rast/Rasterizer.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>

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

} // namespace rast
