#include <rast/Window.hpp>

#include <SDL3/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <algorithm>
#include <cmath>
#include <format>

namespace rast
{

// The values of Key must match SDL's key codes.
static_assert(static_cast<SDL_Keycode>(Key::F1) == SDLK_F1);
static_assert(static_cast<SDL_Keycode>(Key::F2) == SDLK_F2);
static_assert(static_cast<SDL_Keycode>(Key::F3) == SDLK_F3);
static_assert(static_cast<SDL_Keycode>(Key::F4) == SDLK_F4);

#ifdef __EMSCRIPTEN__
// Waits until the browser is about to draw its next frame. This is the browser's VSync.
EM_ASYNC_JS(void, WaitForAnimationFrame, (), {
    await new Promise(resolve => requestAnimationFrame(resolve));
});
#endif

std::expected<Window, std::string> Window::Create(std::string_view title, int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        return std::unexpected(std::format("SDL_Init failed: {}", SDL_GetError()));
    }

    Window window;
    window.m_Title = title;

    window.m_Window.reset(SDL_CreateWindow(window.m_Title.c_str(), width, height, SDL_WINDOW_RESIZABLE));
    if (!window.m_Window)
    {
        std::string error = std::format("SDL_CreateWindow failed: {}", SDL_GetError());
        SDL_Quit();
        return std::unexpected(std::move(error));
    }

    window.m_Renderer.reset(SDL_CreateRenderer(window.m_Window.get(), nullptr));
    if (!window.m_Renderer)
    {
        std::string error = std::format("SDL_CreateRenderer failed: {}", SDL_GetError());
        window.m_Window.reset();
        SDL_Quit();
        return std::unexpected(std::move(error));
    }

    // Wait for the monitor's refresh, so the animation does not tear.
    window.SetVSync(true);

    window.m_FrameTimerStart = SDL_GetTicksNS();

    return window;
}

Window::~Window()
{
    // A moved-from Window owns nothing, so it must not shut SDL down.
    if (m_Window)
    {
        m_Texture.reset();
        m_Renderer.reset();
        m_Window.reset();
        SDL_Quit();
    }
}

bool Window::PollEvents()
{
    m_PressedKeys.clear();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            return false;
        }

        // Ignore the repeated key presses of a key that is held down.
        if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat)
        {
            continue;
        }

        // Remember every key, so the program can ask for it with WasKeyPressed.
        m_PressedKeys.push_back(static_cast<Key>(event.key.key));

        const bool alt = (event.key.mod & SDL_KMOD_ALT) != 0;
        if (event.key.key == SDLK_F11 || (event.key.key == SDLK_RETURN && alt))
        {
            SetFullscreen(!IsFullscreen());
        }
        else if (event.key.key == SDLK_V)
        {
            SetVSync(!m_VSync);
        }
        else if (event.key.key == SDLK_ESCAPE)
        {
            if (!IsFullscreen())
            {
                return false;
            }
            SetFullscreen(false);
        }
    }
    return true;
}

void Window::Present(const Surface& surface)
{
    // Create the texture on the first call, and again when the surface's size changes.
    if (!m_Texture || surface.Width() != m_TextureWidth || surface.Height() != m_TextureHeight)
    {
        m_Texture.reset(SDL_CreateTexture(m_Renderer.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                          surface.Width(), surface.Height()));
        if (!m_Texture)
        {
            SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
            return;
        }

        m_TextureWidth = surface.Width();
        m_TextureHeight = surface.Height();

        // Keep the pixels sharp when the window is larger than the surface,
        // and keep the surface's aspect ratio by adding black bars.
        SDL_SetTextureScaleMode(m_Texture.get(), SDL_SCALEMODE_NEAREST);
        SDL_SetRenderLogicalPresentation(m_Renderer.get(), m_TextureWidth, m_TextureHeight,
                                         SDL_LOGICAL_PRESENTATION_LETTERBOX);
    }

    // Copy the surface into the texture. The pitch is the size of one row of
    // the surface, in bytes. Our rows have no padding, so it's the width times 4.
    const int pitch = surface.Width() * static_cast<int>(sizeof(Color));
    if (!SDL_UpdateTexture(m_Texture.get(), nullptr, surface.Data().data(), pitch))
    {
        SDL_Log("SDL_UpdateTexture failed: %s", SDL_GetError());
    }

    SDL_RenderClear(m_Renderer.get());
    SDL_RenderTexture(m_Renderer.get(), m_Texture.get(), nullptr, nullptr);
    SDL_RenderPresent(m_Renderer.get());

#ifdef __EMSCRIPTEN__
    // Give control back to the browser, so it can show the frame.
    if (m_VSync)
    {
        WaitForAnimationFrame();
    }
    else
    {
        emscripten_sleep(0);
    }
#endif

    // Count the frames, and update the frame rate in the title twice per second.
    ++m_FrameCount;
    const std::uint64_t now = SDL_GetTicksNS();
    const std::uint64_t elapsed = now - m_FrameTimerStart;
    if (elapsed >= SDL_NS_PER_SECOND / 2)
    {
        m_FramesPerSecond = m_FrameCount * static_cast<double>(SDL_NS_PER_SECOND) / static_cast<double>(elapsed);
        m_FrameCount = 0;
        m_FrameTimerStart = now;
        UpdateTitle();
    }
}

void Window::SetVSync(bool enabled)
{
#ifdef __EMSCRIPTEN__
    // In the browser, Present waits for the browser's next frame instead.
    m_VSync = enabled;
#else
    if (SDL_SetRenderVSync(m_Renderer.get(), enabled ? 1 : SDL_RENDERER_VSYNC_DISABLED))
    {
        m_VSync = enabled;
    }
    else
    {
        SDL_Log("SDL_SetRenderVSync failed: %s", SDL_GetError());
    }
#endif
    UpdateTitle();
}

void Window::SetFullscreen(bool fullscreen)
{
    // SDL uses the desktop's resolution, so the monitor doesn't switch modes.
    if (!SDL_SetWindowFullscreen(m_Window.get(), fullscreen))
    {
        SDL_Log("SDL_SetWindowFullscreen failed: %s", SDL_GetError());
    }
}

bool Window::IsFullscreen() const
{
    return (SDL_GetWindowFlags(m_Window.get()) & SDL_WINDOW_FULLSCREEN) != 0;
}

bool Window::WasKeyPressed(Key key) const
{
    return std::ranges::contains(m_PressedKeys, key);
}

Point Window::MousePosition() const
{
    // SDL gives the position in window coordinates...
    float windowX = 0.0f;
    float windowY = 0.0f;
    SDL_GetMouseState(&windowX, &windowY);

    // ...and the renderer converts it to surface coordinates, undoing the
    // scaling and the black bars of SDL_SetRenderLogicalPresentation.
    float x = 0.0f;
    float y = 0.0f;
    SDL_RenderCoordinatesFromWindow(m_Renderer.get(), windowX, windowY, &x, &y);

    // The pixel that contains the point (x, y).
    return { static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)) };
}

void Window::UpdateTitle()
{
    const std::string title =
        std::format("{} | {:.0f} fps | VSync {}", m_Title, m_FramesPerSecond, m_VSync ? "on" : "off");
    SDL_SetWindowTitle(m_Window.get(), title.c_str());
}

void Window::SdlDeleter::operator()(SDL_Window* window) const { SDL_DestroyWindow(window); }
void Window::SdlDeleter::operator()(SDL_Renderer* renderer) const { SDL_DestroyRenderer(renderer); }
void Window::SdlDeleter::operator()(SDL_Texture* texture) const { SDL_DestroyTexture(texture); }

} // namespace rast
