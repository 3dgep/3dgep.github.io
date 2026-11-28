#pragma once

#include "Surface.hpp"

#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <string_view>

// Forward declarations, so this header does not need to include SDL.
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace rast
{

// An SDL3 window that shows a Surface on screen.
//
// Keys:
//   F11 or Alt+Enter  toggle fullscreen
//   V                 toggle VSync
//   Esc               leave fullscreen, or close the window
class Window
{
public:
    // Opens a window, or returns a message that says why it failed.
    static std::expected<Window, std::string> Create(std::string_view title, int width, int height);

    Window(Window&&) noexcept = default;
    Window& operator=(Window&&) = delete;
    ~Window();

    // Handles the window's events. Returns false when the user closes it.
    bool PollEvents();

    // Copies the surface into the window's texture and shows it.
    void Present(const Surface& surface);

    // VSync makes Present wait for the screen's next refresh.
    void SetVSync(bool enabled);
    bool GetVSync() const { return m_VSync; }

    // Fullscreen covers the whole screen, at the desktop's resolution.
    void SetFullscreen(bool fullscreen);
    bool IsFullscreen() const;

private:
    Window() = default;

    // Shows the frame rate and the VSync state in the title bar.
    void UpdateTitle();

    struct SdlDeleter
    {
        void operator()(SDL_Window* window) const;
        void operator()(SDL_Renderer* renderer) const;
        void operator()(SDL_Texture* texture) const;
    };

    // Members are destroyed in reverse order: the texture first, the window last.
    std::unique_ptr<SDL_Window, SdlDeleter> m_Window;
    std::unique_ptr<SDL_Renderer, SdlDeleter> m_Renderer;
    std::unique_ptr<SDL_Texture, SdlDeleter> m_Texture;

    int m_TextureWidth = 0;
    int m_TextureHeight = 0;

    std::string m_Title;
    bool m_VSync = false;

    // For the frame rate: the frames since m_FrameTimerStart (in nanoseconds).
    int m_FrameCount = 0;
    std::uint64_t m_FrameTimerStart = 0;
    double m_FramesPerSecond = 0.0;
};

} // namespace rast
