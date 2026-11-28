#include <rast/Surface.hpp>

#include <cassert>
#include <cstddef>

namespace rast
{

Surface::Surface(int width, int height)
    : m_Width{ width }
    , m_Height{ height }
    , m_Pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height))
{
    assert(width > 0 && height > 0);
}

} // namespace rast
