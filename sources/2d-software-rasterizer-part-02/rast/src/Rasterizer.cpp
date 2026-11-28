#include <rast/Rasterizer.hpp>

#include <algorithm>

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

} // namespace rast
