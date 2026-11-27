// Template, BUAS version https://www.buas.nl/games
// IGAD/BUAS(NHTV)/UU - Jacco Bikker - 2006-2020
// CMGT/BUAS - Jeremiah van Oosten - 2021-2026

// This version of the template uses SDL3 to open the window, show the frame buffer and read the input.
// See: https://www.libsdl.org
//
// SDL runs the main loop and calls the functions below: SDL_AppInit once at the start, SDL_AppEvent for
// every input event, SDL_AppIterate once per frame, and SDL_AppQuit at the end. This way the same code
// runs on Windows and in a web browser (see build-web.bat).

#define SDL_MAIN_USE_CALLBACKS
#include "template.hpp"
#include "surface.hpp"
#include "game.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

// In a browser, the whole page goes fullscreen (the canvas fills the page, see web-shell.html), with the browser's
// Fullscreen API. If the browser refuses (e.g. on a page that doesn't allow it), nothing changes.
EM_JS( void, Tmpl8_SetPageFullscreen, (bool fullscreen), {
	if (fullscreen && !document.fullscreenElement) document.documentElement.requestFullscreen().catch(() => {});
	if (!fullscreen && document.fullscreenElement) document.exitFullscreen().catch(() => {});
} );
EM_JS( bool, Tmpl8_IsPageFullscreen, (), {
	return !!document.fullscreenElement;
} );
// A browser only allows fullscreen in response to a key press or a click: wait for the first one.
EM_JS( void, Tmpl8_FullscreenOnFirstInput, (), {
	const request = () => { if (!document.fullscreenElement) document.documentElement.requestFullscreen().catch(() => {}); };
	window.addEventListener('keydown', request, { once: true, capture: true });
	window.addEventListener('pointerdown', request, { once: true, capture: true });
} );
#endif

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>

namespace Tmpl8 {

namespace {

SDL_Window* window = nullptr; // the game window (owned by SDL)

std::mt19937& RandomGenerator()
{
	static std::mt19937 generator{ std::random_device{}() };
	return generator;
}

} // namespace

float Rand( float range )
{
	if (range <= 0.0f) return 0.0f;
	return std::uniform_real_distribution<float>( 0.0f, range )( RandomGenerator() );
}

int IRand( int range )
{
	if (range <= 0) return 0;
	return std::uniform_int_distribution<int>( 0, range - 1 )( RandomGenerator() );
}

void NotifyUser( const std::string& message )
{
	std::cerr << "ERROR: " << message << '\n';
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, TemplateVersion, message.c_str(), window );
	std::exit( 1 );
}

void SetFullscreen( bool fullscreen )
{
#ifdef __EMSCRIPTEN__
	Tmpl8_SetPageFullscreen( fullscreen );
#else
	// Borderless fullscreen at the desktop resolution; the 800x512 image is scaled to fit (SDL_SCALEMODE_PIXELART).
	if (window) SDL_SetWindowFullscreen( window, fullscreen );
#endif
}

bool IsFullscreen()
{
#ifdef __EMSCRIPTEN__
	return Tmpl8_IsPageFullscreen();
#else
	return window && (SDL_GetWindowFlags( window ) & SDL_WINDOW_FULLSCREEN) != 0;
#endif
}

void ToggleFullscreen()
{
	SetFullscreen( !IsFullscreen() );
}

void SetVSync( bool vsync )
{
	SDL_Renderer* renderer = window ? SDL_GetRenderer( window ) : nullptr;
	if (!renderer) return;
#ifdef __EMSCRIPTEN__
	// In a browser, frames are always shown in step with the screen (requestAnimationFrame).
	(void)vsync;
	SDL_SetRenderVSync( renderer, 1 );
#else
	SDL_SetRenderVSync( renderer, vsync ? 1 : 0 );
#endif
}

bool IsVSync()
{
#ifdef __EMSCRIPTEN__
	return true;
#else
	SDL_Renderer* renderer = window ? SDL_GetRenderer( window ) : nullptr;
	int vsync = 0;
	return renderer && SDL_GetRenderVSync( renderer, &vsync ) && vsync != 0;
#endif
}

void ToggleVSync()
{
	SetVSync( !IsVSync() );
}

#ifndef __EMSCRIPTEN__
// How much larger the window has to be on its display: 1.5 on Windows with 150% display scaling, so the 800x512
// window is 1200x768 pixels there and looks as large as on a display with 100% scaling. (On macOS and Wayland,
// window sizes are already scaled by the system, so this is 1 there.)
static float WindowScale()
{
	const float density = SDL_GetWindowPixelDensity( window );
	const float scale = density > 0 ? SDL_GetWindowDisplayScale( window ) / density : 1.0f;
	return scale > 0 ? scale : 1.0f;
}

// Sizes the window to the 800x512 frame buffer times a_Scale (but not larger than the display).
static void ScaleWindow( float a_Scale )
{
	if (SDL_GetWindowFlags( window ) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED)) return;
	int width = static_cast<int>(ScreenWidth * ScreenScale * a_Scale + 0.5f), height = static_cast<int>(ScreenHeight * ScreenScale * a_Scale + 0.5f);
	SDL_Rect usable;
	if (SDL_GetDisplayUsableBounds( SDL_GetDisplayForWindow( window ), &usable ))
	{
		// keep the shape of the frame buffer, and leave some room for the title bar
		const float fit = Min( 1.0f, Min( usable.w / static_cast<float>(width), (usable.h - 64) / static_cast<float>(height) ) );
		width = static_cast<int>(width * fit), height = static_cast<int>(height * fit);
	}
	SDL_SetWindowSize( window, width, height );
}
#endif

// Everything the template needs while the application runs.
struct Application
{
#ifndef __EMSCRIPTEN__
	float windowScale = 1.0f; // see WindowScale()
#endif
	SDL_Renderer* renderer = nullptr;   // owned by SDL (destroyed in SDL_AppQuit)
	SDL_Texture* frameBuffer = nullptr; // owned by SDL (destroyed in SDL_AppQuit)
	Surface screen{ ScreenWidth, ScreenHeight }; // the game draws to this surface; it is shown after every Tick
	Game game{ screen };
	std::chrono::steady_clock::time_point previousTime;
};

} // namespace Tmpl8

using namespace Tmpl8;

SDL_AppResult SDL_AppInit( void** appstate, int argc, char* argv[] )
{
	std::cout << "application started.\n";

	if (!SDL_Init( SDL_INIT_VIDEO )) NotifyUser( SDL_GetError() );

	auto app = std::make_unique<Application>();

	// The window can be resized (or made fullscreen): the 800x512 frame buffer is scaled to fit.
	// SDL_WINDOW_HIGH_PIXEL_DENSITY: the renderer uses all the pixels of the window (e.g. 1200x768 pixels for an
	// 800x512 window with 150% display scaling), so SDL_SCALEMODE_PIXELART does all the scaling and keeps it sharp.
	SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#ifdef __EMSCRIPTEN__
	if constexpr (StartFullscreen) Tmpl8_FullscreenOnFirstInput();
#else
	if constexpr (StartFullscreen) flags |= SDL_WINDOW_FULLSCREEN;
#endif
#ifdef __EMSCRIPTEN__
	// In a browser, the page (web-shell.html) makes the canvas fill the whole browser window and SDL follows its size.
	// With SDL_WINDOW_HIGH_PIXEL_DENSITY, the canvas gets as many pixels as the screen has, so the browser doesn't
	// scale it again.
	// SDL_WINDOW_OPENGL: without it, SDL 3.4.2 (the version in Emscripten) re-creates the window for the renderer
	// and the canvas ends up 1x1 pixels. (SDL_WINDOW_FILL_DOCUMENT doesn't work in that version: the canvas stays 0x0.)
	flags |= SDL_WINDOW_OPENGL;
#endif
	if (!SDL_CreateWindowAndRenderer( TemplateVersion, ScreenWidth * ScreenScale, ScreenHeight * ScreenScale, flags, &window, &app->renderer )) NotifyUser( SDL_GetError() );
#ifndef __EMSCRIPTEN__
	// On a display with e.g. 150% scaling, make the window 1.5x as large, and keep it in the centre of the display.
	app->windowScale = WindowScale();
	if (app->windowScale != 1.0f)
	{
		ScaleWindow( app->windowScale );
		SDL_SetWindowPosition( window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
	}
#endif
	SetVSync( StartVSync );
	SDL_SetRenderLogicalPresentation( app->renderer, ScreenWidth, ScreenHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX );
	app->frameBuffer = SDL_CreateTexture( app->renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, ScreenWidth, ScreenHeight );
	if (!app->frameBuffer) NotifyUser( SDL_GetError() );
	SDL_SetTextureScaleMode( app->frameBuffer, SDL_SCALEMODE_PIXELART );

	app->screen.Clear( 0 );
	app->game.Init();
	app->previousTime = std::chrono::steady_clock::now();
	// SDL keeps the application in a void* (SDL is written in C) and passes it to the other callbacks;
	// SDL_AppQuit takes it back and deletes it.
	*appstate = app.release();
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent( void* appstate, SDL_Event* event )
{
	auto* app = static_cast<Application*>(appstate);
	// Mouse positions in frame buffer pixels, also when the window is scaled.
	SDL_ConvertEventToRenderCoordinates( app->renderer, event );
	switch (event->type)
	{
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;
#ifndef __EMSCRIPTEN__
	case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
	{
		// The window moved to a display with a different scaling: keep it the same size on screen.
		const float scale = WindowScale();
		if (scale != app->windowScale)
		{
			int width, height;
			SDL_GetWindowSize( window, &width, &height );
			const float factor = scale / app->windowScale;
			app->windowScale = scale;
			if (!(SDL_GetWindowFlags( window ) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED)))
				SDL_SetWindowSize( window, static_cast<int>(width * factor + 0.5f), static_cast<int>(height * factor + 0.5f) );
		}
		break;
	}
#endif
	case SDL_EVENT_KEY_DOWN:
#ifndef __EMSCRIPTEN__ // in a browser, Escape leaves fullscreen instead
		if (event->key.key == SDLK_ESCAPE) return SDL_APP_SUCCESS;
#endif
		// Alt+Enter or F11: switch between fullscreen and a window.
		if (!event->key.repeat && (event->key.key == SDLK_F11 || (event->key.key == SDLK_RETURN && (event->key.mod & SDL_KMOD_ALT))))
		{
			ToggleFullscreen();
			break;
		}
		if (!event->key.repeat) app->game.KeyDown( event->key.scancode ); // ignore the repeats while a key is held down
		break;
	case SDL_EVENT_KEY_UP:
		app->game.KeyUp( event->key.scancode );
		break;
	case SDL_EVENT_MOUSE_MOTION:
		app->game.MouseMove( static_cast<int>(event->motion.x), static_cast<int>(event->motion.y) );
		break;
	case SDL_EVENT_MOUSE_BUTTON_UP:
		app->game.MouseUp( event->button.button );
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		app->game.MouseDown( event->button.button );
		break;
	default:
		break;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate( void* appstate )
{
	auto* app = static_cast<Application*>(appstate);

	// Time since the previous frame, in milliseconds.
	const auto currentTime = std::chrono::steady_clock::now();
	const float deltaTime = std::chrono::duration<float, std::milli>( currentTime - app->previousTime ).count();
	app->previousTime = currentTime;

	app->game.ProcessInput();
	app->game.Tick( deltaTime );

	SDL_UpdateTexture( app->frameBuffer, nullptr, app->screen.GetBuffer(), app->screen.GetPitch() * static_cast<int>(sizeof( Pixel )) );
	SDL_RenderClear( app->renderer );
	SDL_RenderTexture( app->renderer, app->frameBuffer, nullptr, nullptr );
	SDL_RenderPresent( app->renderer );
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit( void* appstate, SDL_AppResult result )
{
	const std::unique_ptr<Application> app( static_cast<Application*>(appstate) ); // deleted at the end of this function
	if (!app) return;
	app->game.Shutdown();
	SDL_DestroyTexture( app->frameBuffer );
	SDL_DestroyRenderer( app->renderer );
	SDL_DestroyWindow( window );
	window = nullptr;
	// SDL calls SDL_Quit after this function.
}
